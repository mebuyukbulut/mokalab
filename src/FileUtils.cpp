#include "FileUtils.h"
// #include <windows.h>
// #include <commdlg.h>
#include <fstream>
#include <algorithm>
#include "Logger.h"
//#include <chrono>
//#include <thread>

// read the contents of a file and return it as a string
std::string FileUtils::readFile(const std::string& filePath)
{
	std::ifstream infile{ filePath };
	std::string file_contents{ std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };
    return file_contents;
}

void FileUtils::writeFile(const std::string &filePath, const std::string &str)
{
	std::ofstream fout(filePath);
	fout << str;
    fout.close();
}

// https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exists-using-standard-c-c11-14-17-c
// Control the file is exists or not 
bool FileUtils::isExists(const std::string& name) {
    std::ifstream f(name.c_str()); 
    return f.good();
}


// esat@fedora:~/dev/graphics/mokalab/build$ zenity --help-file-selection
// Usage:
//   zenity [OPTION…]// 

// File selection options
//   --file-selection                               Display file selection dialog
//   --filename=FILENAME                            Set the filename
//   --multiple                                     Allow multiple files to be selected
//   --directory                                    Activate directory-only selection
//   --save                                         Activate save mode
//   --separator=SEPARATOR                          Set output separator character
//   --file-filter=NAME | PATTERN1 PATTERN2 ...     Set a filename filter
//   --confirm-overwrite                            DEPRECATED; does nothing

std::string FileUtils::openFileDialog(const wchar_t* filter, bool isSaveMode, bool directoryOnly) {
    //LOG_ERROR("FileUtils is OS dependent!");
    char filename[1024];
    FILE *f{};

    if(!directoryOnly){
        if(isSaveMode)
            f = popen("zenity --file-selection --save", "r");
        else
            f = popen("zenity --file-selection", "r");
    }
    else
    {
        f = popen("zenity --file-selection --directory", "r");
    }
    

    if (!f) {
        LOG_ERROR("Zenity başlatılamadı!");
        return "";
    }
    
    // Kullanıcı iptal ederse fgets nullptr döner
    if (fgets(filename, sizeof(filename), f) == nullptr) {
        pclose(f);
        return ""; // Seçim yapılmadı
    }

    pclose(f);


    std::string path(filename);
    trimLeft(path);
    trimRight(path);
    return path;
    // OPENFILENAME ofn;       // Common dialog box structure
    // wchar_t szFile[260] = { 0 }; // Buffer for file name
    // ZeroMemory(&ofn, sizeof(ofn));
    // ofn.lStructSize = sizeof(ofn);
    // ofn.hwndOwner = nullptr; // Or your window handle
    // ofn.lpstrFile = szFile;
    // ofn.nMaxFile = sizeof(szFile);
    // //wchar_t* ptr = const_cast<wchar_t*>(filter);
    // ofn.lpstrFilter = filter;//filter;
    // ofn.nFilterIndex = 1;
    // ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    // if (GetOpenFileName(&ofn) == TRUE) {
    //     int size_needed = WideCharToMultiByte(CP_UTF8, 0, szFile, -1, nullptr, 0, nullptr, nullptr);
    //     std::string utf8Path(size_needed - 1, 0); // exclude null terminator
    //     WideCharToMultiByte(CP_UTF8, 0, szFile, -1, &utf8Path[0], size_needed, nullptr, nullptr);
    //     return utf8Path;

    //     //return ofn.lpstrFile;
    // }
    //return {};
}

void FileUtils::createDirectory(const std::filesystem::path &newDirectoryPath)
{
    std::filesystem::create_directory(newDirectoryPath);
}

std::wstring FileUtils::UTF8ToWString(const std::string& str)
{
    LOG_ERROR("FileUtils is OS dependent!");
    // if (str.empty()) return std::wstring();
    // int size_needed = MultiByteToWideChar(CP_UTF8, 0,
    //     str.data(), (int)str.size(),
    //     nullptr, 0);
    // std::wstring wstrTo(size_needed, 0);
    // MultiByteToWideChar(CP_UTF8, 0,
    //     str.data(), (int)str.size(),
    //     &wstrTo[0], size_needed);
    // return wstrTo;

    return {};
}

std::wstring FileUtils::ANSIToWString(const std::string& str)
{
    LOG_ERROR("FileUtils is OS dependent!");
    // if (str.empty()) return std::wstring();
    // int size_needed = MultiByteToWideChar(CP_ACP, 0, // CP_ACP = ANSI code page
    //     str.data(), (int)str.size(),
    //     nullptr, 0)+1;
    // std::wstring wstrTo(size_needed, 0);
    // MultiByteToWideChar(CP_ACP, 0,
    //     str.data(), (int)str.size(),
    //     &wstrTo[0], size_needed);
    // return wstrTo;
    return {};
}

inline void FileUtils::trimLeft(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), 
    [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

inline void FileUtils::trimRight(std::string &s)
 {
    s.erase(std::find_if(s.rbegin(), s.rend(), 
    [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}
