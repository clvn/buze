#include <windows.h>
#include <wincrypt.h>
#include <vector>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>

using std::cout;
using std::endl;

HCRYPTHASH create_hash_from_string(HCRYPTPROV hProv, const char* str) {
	HCRYPTHASH hHash;
	BOOL bResult = CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash);
	if (!bResult) return 0;

	bResult = CryptHashData(hHash, (BYTE*)str, (DWORD)strlen(str), 0);  
	if (!bResult) {
		CryptDestroyHash(hHash);
		return 0;
	}
	return hHash;
}


HCRYPTHASH create_hash_from_file(HCRYPTPROV hProv, const char* filename) {

	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE == hFile) 
		return 0;

	HCRYPTHASH hHash;
	BOOL bResult = CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash);
	if (!bResult) {
		CloseHandle(hFile);
		return 0;
	}

	const int BUFSIZE = 1024;
	BYTE pcBuffer[BUFSIZE];
	DWORD dwReadBytes;

	while (bResult = ReadFile(hFile, pcBuffer, BUFSIZE, &dwReadBytes, NULL)) {
		if (0 == dwReadBytes)
			break;

		bResult = CryptHashData(hHash, pcBuffer, dwReadBytes, 0);
		if (!bResult) {
			CryptDestroyHash(hHash);
			hHash = 0;
			break;
		}
	}
	
	CloseHandle(hFile);
	return hHash;
}



bool get_hash_bytes(HCRYPTHASH hHash, std::vector<unsigned char>& result) {
	DWORD dwBufferSize = sizeof(DWORD);
	DWORD dwSize;
	BOOL bResult = CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&dwSize, &dwBufferSize, 0);    
	if (!bResult) return false;

	result.resize(dwSize);

	// Get hash value.
	bResult = CryptGetHashParam(hHash, HP_HASHVAL, (BYTE*)&result.front(), &dwSize, 0);
	if (!bResult) return false;

	return true;
}

bool get_key_bytes(HCRYPTKEY hKey, DWORD dwBlobType, std::vector<unsigned char>& result) {

	DWORD dwBlobLength;
	BOOL bResult = CryptExportKey(hKey, NULL, dwBlobType, 0, NULL, &dwBlobLength);
	if (!bResult) return false;

	result.resize(dwBlobLength);

	bResult = CryptExportKey(hKey, NULL, dwBlobType, 0, (BYTE*)&result.front(), &dwBlobLength);
	if (!bResult) return false;

	return true;
}

void print_c_blob(std::ostream& strm, const std::string& name, std::vector<unsigned char>& bytes) {

	strm << "unsigned char " << name << "[] = {";
	for (std::vector<unsigned char>::iterator i = bytes.begin(); i != bytes.end(); ++i) {
		int index = (int)std::distance(bytes.begin(), i);
		if (index > 0)
			strm << ", ";
		if (index % 16 == 0)
			strm << endl << "    ";
		strm << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)*i;
	}
	strm << endl << "};" << endl;
}

void print_hexstream(std::ostream& strm, std::vector<unsigned char>& bytes) {
	for (std::vector<unsigned char>::iterator i = bytes.begin(); i != bytes.end(); ++i) {
		strm << std::hex << std::setw(2) << std::setfill('0') << (int)*i;
	}
}

void generate_and_print_keys(HCRYPTPROV hProv, std::ostream& strm) {

	// NOTE: CryptDeriveKey cannot be used to generate public/private keys
	// use CryptGenKey for this
	HCRYPTKEY hKey;
	BOOL bResult = CryptGenKey(hProv, AT_SIGNATURE, CRYPT_EXPORTABLE, &hKey);

	std::vector<unsigned char> privateblob;
	get_key_bytes(hKey, PRIVATEKEYBLOB, privateblob);

	std::vector<unsigned char> publicblob;
	get_key_bytes(hKey, PUBLICKEYBLOB, publicblob);

	print_c_blob(strm, "publickey", publicblob);
	print_c_blob(strm, "privatekey", privateblob);

	CryptDestroyKey(hKey);
}

bool parse_hexstring(const std::string& str, std::vector<unsigned char>& result) {
	if ((str.empty() || str.length() % 2) != 0) return false;
	int c;
	result.resize(str.length() / 2);
	for (int i = 0; i < (int)str.length() / 2; i++) {
		std::istringstream strm(str.substr(i * 2, 2));
		strm >> std::hex >> c;
		if (!strm) return false;
		result[i] = c & 0xFF;
	}
	return true;
}

/*extern unsigned char publickey[];
extern size_t sizeof_publickey;

bool verify_signature(const std::string& hexsig, std::string filename) {
	std::vector<unsigned char> signature;
	if (!parse_hexstring(hexsig, signature))
		return false;

	HCRYPTPROV hProv;

	BOOL bResult = CryptAcquireContext(&hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	if (!bResult) return false;

	HCRYPTKEY hPublicKey;
	bResult = CryptImportKey(hProv, publickey, sizeof_publickey, 0, 0, &hPublicKey);
	if (!bResult) {
		CryptReleaseContext(hProv, 0);
		return false;
	}

	HCRYPTHASH hFileHash = create_hash_from_file(hProv, filename.c_str());
	if (!hFileHash) {
		CryptDestroyKey(hPublicKey);
		CryptReleaseContext(hProv, 0);
		return false;
	}

	bResult = CryptVerifySignature(hFileHash, &signature.front(), (DWORD)signature.size(), hPublicKey, "", 0);

	CryptDestroyHash(hFileHash);
	CryptDestroyKey(hPublicKey);
	CryptReleaseContext(hProv, 0);
	return bResult != 0;
}
*/