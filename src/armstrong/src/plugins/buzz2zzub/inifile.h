#pragma once

namespace ini {

struct section {
	std::string fileName;
	std::string sectionName;

	template <typename T>
	T get(std::string name, T defaultValue);

	template<>
	int get(std::string name, int defaultValue) {
		char defaultStr[16];
		itoa(defaultValue, defaultStr, 10);
		char result[1024];
		GetPrivateProfileString(sectionName.c_str(), name.c_str(), defaultStr, result, 1024, fileName.c_str());
		return atoi(result);
	}
	template<>
	std::string get(std::string name, std::string defaultValue) {
		if (defaultValue == "") defaultValue = name;
		char result[1024];
		GetPrivateProfileString(sectionName.c_str(), name.c_str(), defaultValue.c_str(), result, 1024, fileName.c_str());
		return result;
	}
};

struct file {
	std::string fileName;	// MSDN: If this parameter does not contain a full path to the file, the system searches for the file in the Windows directory. 

	file(std::string fn) { fileName = fn; }
	ini::section section(std::string sectionName) {
		ini::section inis = { fileName, sectionName };
		return inis;
	}
};

}

