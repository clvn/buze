#include <atlbase.h>
#include <atlapp.h>
#include <atlconv.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <windows.h>

#include "../vstsdk2.4/pluginterfaces/vst2.x/aeffectx.h"

using std::wcout;
using std::cout;
using std::endl;
using std::wcerr;

typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);
static VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);

VstIntPtr VSTCALLBACK HostCallback (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
/*
	ONLY the following audioMaster callbacks are allowed before the initial DLL-call "main()" or "VstPluginMain()" returns to host:
	{01} audioMasterVersion
	{06} audioMasterWantMidi ... deprecated in VST2.4
	{32} audioMasterGetVendorString
	{33} audioMasterGetProductString
	{34} audioMasterGetVendorVersion
	{38} audioMasterGetLanguage
	{41} audioMasterGetDirectory
*/
	VstIntPtr result = 0;
	switch(opcode) {
		case audioMasterVersion:
			return kVstVersion;
		//case audioMasterNeedIdle:
		//	cout << "ignoring audioMasterNeedIdle" << endl;
		//	return 0;
	}
	return 0;
}

AEffect* CreateVstEffect(std::wstring path, HMODULE* hPluginResult) {

	HMODULE hPlugin = LoadLibrary(path.c_str());
	if (!hPlugin) {
		return 0;
	}

	wcerr << path << endl;
	PluginEntryProc mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)hPlugin, "VSTPluginMain");
	if (!mainProc) 
		mainProc = (PluginEntryProc)GetProcAddress ((HMODULE)hPlugin, "main");

	if (!mainProc) {
		FreeLibrary(hPlugin);
		return 0;
	}

	AEffect* effect = mainProc(HostCallback);
	if(!effect) {
		FreeLibrary(hPlugin);
		return 0;
	}

	if (hPluginResult)
		*hPluginResult = hPlugin;
	return effect;
}

void Indent(int count, std::wstringstream& output) {
	for (int i = 0; i < count; i++) {
		output << L" ";
	}
}

void ScanFiles(int indent, std::wstring path, std::wstringstream& output) {

	USES_CONVERSION;

	std::wstringstream strm;
	strm << path << "\\*.dll";

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(strm.str().c_str(), &fd);
	//HANDLE hFind = FindFirstFile(L"Shapes\\*.json", &fd);
	while (hFind != INVALID_HANDLE_VALUE) {
		
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			std::wstring filename = fd.cFileName;
			size_t ld = filename.find_last_of('.');
			std::wstring ext = filename.substr(ld + 1);
			std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
			if (ext == L"dll") {
				HMODULE hPlugin;
				AEffect* effect = CreateVstEffect(path  + L"\\" + filename, &hPlugin);
				if (effect) {
					effect->dispatcher (effect, effOpen, 0, 0, 0, 0);
					std::wstringstream uristrm;
					uristrm << L"@zzub.org/vst/" << std::hex << std::setw(8) << std::setfill(L'0') << effect->uniqueID;
					std::wstring uri = uristrm.str();

					std::wstring baseName, productName, effectName;
					{
						char pcEffectName[256] = {0};
						char pcVendorString[256] = {0};
						char pcProductString[256] = {0};

						effect->dispatcher (effect, effGetEffectName, 0, 0, pcEffectName, 0);
						//effect->dispatcher (effect, effGetVendorString, 0, 0, pcVendorString, 0);
						effect->dispatcher (effect, effGetProductString, 0, 0, pcProductString, 0);
	
						baseName = filename.substr(0, ld);
						effectName = A2W(pcEffectName);
						productName = A2W(pcProductString);
					}

					std::wstring vstName;
					if (effectName == productName) {
						vstName = productName;
					} else if (effectName.empty())
						vstName = productName;
					else if (productName.empty())
						vstName = effectName;
					else
						vstName = productName + L" " + effectName;

					if (vstName.empty()) vstName = baseName;

					Indent(indent * 4, output);
					output << uri << L", " << vstName << endl;
					effect->dispatcher (effect, effClose, 0, 0, 0, 0);
					FreeLibrary(hPlugin);
				}
			}
		}
		if (!FindNextFile(hFind, &fd)) break;
	}
	FindClose(hFind);

}

void ScanFolder(int indent, std::wstring path, std::wstringstream& output) {

	USES_CONVERSION;

	std::wstringstream strm;
	strm << path << "\\*.*";

	ScanFiles(indent, path, output);

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(strm.str().c_str(), &fd);
	//HANDLE hFind = FindFirstFile(L"Shapes\\*.json", &fd);
	while (hFind != INVALID_HANDLE_VALUE) {
		
		if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {

			std::wstring dirname = fd.cFileName;
			if (dirname != L"." && dirname != L"..") {
				std::wstringstream folderoutput;
				ScanFolder(indent + 1, path  + L"\\" + dirname, folderoutput);
				if (folderoutput.tellp() > 0) {
					Indent(indent * 4, output);
					output << L"/" << dirname << endl;
					output << folderoutput.str();
					Indent((indent + 1) * 4, output);
					output << L"/.." << endl;
				}
			}
		}

		if (!FindNextFile(hFind, &fd)) break;
	}
	FindClose(hFind);
}

int main(int argc, char** argv) {

	USES_CONVERSION;

	if (argc != 2) {
		wcout << L"usage: vst2index [path to vsts]" << endl;
		return 1;
	}

	std::wstringstream output;
	ScanFolder(0, A2W(argv[1]), output);
	//ScanFolder(0, _T("c:/code/buze/trunk/buze/gear/vst"), output);

	wcout << output.str() << endl;
	return 0;
}
