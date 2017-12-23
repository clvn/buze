#pragma once

HCRYPTHASH create_hash_from_string(HCRYPTPROV hProv, const char* str);
HCRYPTHASH create_hash_from_file(HCRYPTPROV hProv, const char* filename);
bool get_hash_bytes(HCRYPTHASH hHash, std::vector<unsigned char>& result);
bool get_key_bytes(HCRYPTKEY hKey, DWORD dwBlobType, std::vector<unsigned char>& result);
void print_c_blob(std::ostream& strm, const std::string& name, std::vector<unsigned char>& bytes);
void print_hexstream(std::ostream& strm, std::vector<unsigned char>& bytes);
void generate_and_print_keys(HCRYPTPROV hProv, std::ostream& strm);
bool parse_hexstring(const std::string& str, std::vector<unsigned char>& result);

