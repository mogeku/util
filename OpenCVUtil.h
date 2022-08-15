//
// Created by Lee on 2022/1/10.
//

#ifndef LIB_IMAGEDUPLICATE_SDK_OPENCVUTIL_H
#define LIB_IMAGEDUPLICATE_SDK_OPENCVUTIL_H

#include <string>
#include <locale>

#include "opencv2/opencv.hpp"

namespace util {
namespace opencv {

inline bool IsAllZero(const cv::Mat &img) {
    bool ret = true;
    try {
        img.forEach<uchar>([&ret](uchar val, const int *position) {
            if (val != 0) ret = false;
        });
        return ret;
    }
    catch (...) {
        return false;
    }
}

#include <fstream>
inline cv::Mat ImreadUtf8(const std::string &path, int flag = cv::IMREAD_COLOR) {
    try {
        cv::Mat buf;
        fs::path p = fs::u8path(path);

        if (util::file::IsLongPath(path)) {
#ifdef _WIN32
            HANDLE hd = CreateFileW((L"\\\\?\\" + p.wstring()).c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
                                    0, 0);
            if (hd == INVALID_HANDLE_VALUE) return cv::Mat();

            auto size = GetFileSize(hd, NULL);
            buf.create(1, size, CV_8UC1);
            ReadFile(hd, buf.data, size, NULL, NULL);
            CloseHandle(hd);
#endif
        } else {
            std::ifstream ifs(p, std::ios::binary);
            if (!ifs.is_open()) return cv::Mat();

            auto size = static_cast<int>(util::file::getFileSize(ifs));
            buf.create(1, size, CV_8UC1);
            ifs.read(reinterpret_cast<char *>(buf.data), size);
        }

        return cv::imdecode(buf, flag);
    }
    catch (...) {
        return cv::Mat();
    }
}

inline bool ImwriteUtf8(const std::string &path, cv::Mat img) {
    try {
        std::string local = setlocale(LC_ALL, NULL);
        setlocale(LC_ALL, ".65001");
        bool ret = cv::imwrite(path, img);

        setlocale(LC_ALL, local.c_str());
        return ret;
    }
    catch (...) {
        return false;
    }
}

inline bool IsImage(const std::string &path) {
    return !ImreadUtf8(path).empty();
}


}   // namespace opencv
}   // namespace util

#endif //LIB_IMAGEDUPLICATE_SDK_OPENCVUTIL_H
