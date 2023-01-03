#pragma once
#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <fstream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <filesystem>
namespace fs = std::experimental::filesystem;
#else
#include "filesystem.hpp"
namespace fs = ghc::filesystem;
#endif

namespace util {
namespace file {

const std::string kPathSeparator =
#ifdef _WIN32
        "\\";
#else
"/";
#endif

// #ifdef _WIN32

// inline std::string Utf16ToUtf8(const std::wstring &u16str) {
//     if (u16str.empty()) {
//         return std::string();
//     }

//     std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
//     return conv.to_bytes(u16str);
// }

// inline std::wstring Utf8ToUtf16(const std::string &u8str) {
//     if (u8str.empty()) {
//         return std::wstring();
//     }

//     std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
//     return conv.from_bytes(u8str);
// }

// #endif //_WIN32

inline bool IsLongPath(const std::string& path) {
    bool ret = false;
#ifdef _WIN32
    ret = path.size() >= MAX_PATH;
#endif
    return ret;
}

inline bool CheckIsFileExist(const std::string &file_path) {
    return fs::exists(fs::u8path(file_path));
}

inline bool CheckIsDir(const fs::path &file_path) {
    return fs::is_directory(file_path);
}

inline bool CheckIsDir(const std::string &file_path) {
    return fs::is_directory(fs::u8path(file_path));
}

inline bool CheckIsFile(const fs::path &file_path) {
    return fs::is_regular_file(file_path);
}

inline bool CheckIsFile(const std::string &file_path) {
    return fs::is_regular_file(fs::u8path(file_path));
}

inline std::fstream OpenFileUtf8(const std::string &file_path, std::ios_base::openmode mode = std::ios::in | std::ios::out) {
    return std::fstream(fs::u8path(file_path), mode);
}

inline uintmax_t Remove(const std::string &path) {
    fs::path p = fs::u8path(path);

    if (fs::is_directory(p))
        return fs::remove_all(p);
    else
        return fs::remove(p);
}

inline uintmax_t Remove(const std::string &path, std::error_code &err_code) {
    fs::path p = fs::u8path(path);

    if (fs::is_directory(p))
        return fs::remove_all(p, err_code);
    else
        return fs::remove(p, err_code);
}

inline bool Mkdir(const std::string &dir_path, bool is_remove=false) {
    if (is_remove) Remove(dir_path);
    return fs::create_directories(fs::u8path(dir_path));
}

inline bool Mkdir(const std::string &dir_path, std::error_code &err_code) {
    return fs::create_directories(fs::u8path(dir_path), err_code);
}

inline bool Touch(const std::string &file_path) {
    if (CheckIsFileExist(file_path))
        return false;

    std::fstream f = OpenFileUtf8(file_path, std::ios::out);
    if (!f.is_open())
        return false;
    f.close();
    return true;
}

inline std::string GetParentPath(const std::string &file_path, char path_sep) {
    std::string ret;
    fs::path p = fs::u8path(file_path);
    ret = p.parent_path().u8string();
    if (path_sep != fs::path::preferred_separator)
        std::replace(ret.begin(), ret.end(), static_cast<char>(p.preferred_separator), path_sep);
    return ret;
}

inline std::string Join(fs::path p1, fs::path p2) {
    return p1.append(p2).u8string();
}


inline uint64_t getFileSize(std::ifstream& fs) {
    uint64_t size = 0;
    if (fs.is_open())
    {
        fs.seekg(0, fs.end);
        size = fs.tellg();
        fs.seekg(0, fs.beg);
    }
    return size;
}

inline uint64_t getFileSize(const std::string &file_path) {
    fs::path p = fs::u8path(file_path);
    try {
        if (!IsLongPath(file_path))
            return fs::file_size(fs::u8path(file_path));
        else {
            uint64_t size = 0;
#ifdef _WIN32
            HANDLE hd = CreateFileW((L"\\\\?\\" + p.wstring()).c_str(), FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING,
                                    0, 0);
            if (hd != INVALID_HANDLE_VALUE) {
                size = GetFileSize(hd, NULL);
                CloseHandle(hd);
            }
#endif
            return size;
        }

    }
    catch (...) {
        return 0;
    }
}

inline std::string GetExtension(const std::string &file_path) {
    auto dot_pos = file_path.find_last_of('.');
    if (std::string::npos == dot_pos)
        return "";
    return file_path.substr(dot_pos);
}

inline bool IsHidden(const std::string &file_path) {
    fs::path p = fs::u8path(file_path);
#ifdef _WIN32
    DWORD attr = 0;
    if (IsLongPath(file_path))
        attr = GetFileAttributesW((L"\\\\?\\" + p.wstring()).c_str());
    else
        attr = GetFileAttributesA((p.string()).c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
        return false;
    return attr & FILE_ATTRIBUTE_HIDDEN;
#else
    std::string filename = p.filename().string();
    return (filename != ".." &&
            filename != "." &&
            filename[0] == '.');
#endif
}

}   // namespace file
}   // namespace util
