#pragma once

#include <string>
#include <filesystem>

class FileUtils
{
public:
	static bool isExists(const std::string& filePath);
	static std::string readFile(const std::string& filePath);
	static void writeFile(const std::string& filePath, const std::string& str);

	static std::string openFileDialog(const wchar_t* filter = L"All Files\0*.*\0", bool isSaveMode = false, bool directoryOnly = false);
	static void createDirectory(const std::filesystem::path& newDirectoryPath);


	static std::wstring UTF8ToWString(const std::string& str);
	static std::wstring ANSIToWString(const std::string& str);

	// Trim from the start (in place)
	static inline void trimLeft(std::string &s);

	// Trim from the end (in place)
	static inline void trimRight(std::string &s);
};


