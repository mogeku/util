/*
 * @Author: momo
 * @Date: 2022-06-09 18:12:18
 * @LastEditors: momo
 * @LastEditTime: 2022-06-13 17:33:52
 * @FilePath: \lib_ImageClassify_ORT_SDK\src\util\StringUtil.h
 * @Description:
 * Copyright (c) 2022 by momo, All Rights Reserved.
 */
#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <stdarg.h>

namespace util {
namespace str {


inline long long Stoll(const std::string &_Str, size_t *_Idx = nullptr,
                       int _Base = 10) {
    try {
        return std::stoll(_Str, _Idx, _Base);
    }
    catch (...) {
        return 0;
    }
}

inline long long Stoll(const std::wstring &_Str, size_t *_Idx = nullptr,
                       int _Base = 10) {
    try {
        return std::stoll(_Str, _Idx, _Base);
    }
    catch (...) {
        return 0;
    }
}

inline std::vector<std::string> Split(const std::string &str, const std::string sep) {
    std::vector<std::string> ret;

    size_t start_pos = 0;
    size_t end_pos = 0;
    while (std::string::npos != (end_pos = str.find(sep, start_pos))) {
        ret.emplace_back(str.substr(start_pos, end_pos - start_pos));
        start_pos = end_pos + sep.size();
    }
    ret.emplace_back(str.substr(start_pos, end_pos));
    return ret;
}

inline std::string Replace(std::string str,
                           const std::string &old_str, const std::string &new_str,
                           size_t off = 0) {
    if (str.empty()) return str;

    for (size_t pos = str.find(old_str, off);
         pos != std::string::npos;
         pos = str.find(old_str, pos + old_str.size())) {
        str.replace(pos, old_str.size(), new_str);
    }
    return str;
}

inline std::string Replace(const std::string &str,
                           const char old_char, const char new_char,
                           size_t off = 0) {
    return Replace(str, std::string(1, old_char), std::string(1, new_char), off);
}

inline bool IsAscii(const std::string &str) {
    for (char c: str) {
        if (!std::isprint(c))
            return false;
    }
    return true;
}

inline std::string Blob2HexStr(const std::vector<unsigned char> &blob) {
    if (blob.empty())
        return "";
    static const char hexArr[] = "0123456789abcdef";
    std::stringstream ss;
    for (unsigned char c: blob) {
        ss << hexArr[(c >> 4) & 15];
        ss << hexArr[c & 15];
    }
    return ss.str();
}

inline std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

inline std::string Sprintf(const char* fmt, ...) {
    char buf[512];
    va_list va;
    va_start(va, fmt);
    int len = vsprintf_s(buf, 512, fmt, va);
    return std::string(buf, len);
}

}   // namespace str
}   // namespace util
