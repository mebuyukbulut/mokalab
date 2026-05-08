#pragma once

#include <string>

class FileUtils
{
public:
	static std::string readFile(const std::string& filePath);
	static std::string openFileDialog(const wchar_t* filter = L"All Files\0*.*\0");

	static std::wstring UTF8ToWString(const std::string& str);
	static std::wstring ANSIToWString(const std::string& str);

	// Trim from the start (in place)
	static inline void trimLeft(std::string &s);

	// Trim from the end (in place)
	static inline void trimRight(std::string &s);
};


