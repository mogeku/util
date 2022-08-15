/*
 * @Author: momo
 * @Date: 2022-06-09 18:12:18
 * @LastEditors: momo
 * @LastEditTime: 2022-06-10 18:08:37
 * @FilePath: \lib_ImageClassify_ORT_SDK\src\util\LogUtil.h
 * @Description:
 * Copyright (c) 2022 by momo, All Rights Reserved.
 */
#pragma once

#include <string>

#include "LogSDK.hpp"

namespace util::log
{
#ifdef _WIN32
const std::string kPathSep = "\\";
#else
const std::string kPathSep = "/";
#endif

const std::string kLogCategory = "imgcls";

#define LOGE(format, ...) LogEx(util::log::kLogCategory.c_str(), LOG_ERROR, "---%s---\n[%s](%d): " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGW(format, ...) LogEx(util::log::kLogCategory.c_str(), LOG_WARN, "---%s---\n[%s](%d): " format, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGI(format, ...) LogEx(util::log::kLogCategory.c_str(), LOG_INFO, "[%s](%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGD(format, ...) LogEx(util::log::kLogCategory.c_str(), LOG_DEBUG, "[%s](%d): " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ASSERT(val, ret, log, ...) \
if (!(val)) { \
    LOGE(log, ##__VA_ARGS__); \
    return ret; \
}

#define ASSERT_WARN(val, log, ...) \
if (!(val)) { \
    LOGW(log, ##__VA_ARGS__); \
}

#define ASSERT_CONTINUE(val, log, ...) \
if (!(val)) { \
    LOGW(log, ##__VA_ARGS__); \
    continue; \
}

#define ASSERT_NOTNULL(val, ret, log, ...) \
if (nullptr == (val)) { \
    LOGE(log, ##__VA_ARGS__); \
    return ret; \
}

inline bool initLog(std::string config_dir, std::string log_dir) {
    std::string log_full_path = log_dir + kPathSep + kLogCategory + ".log";
    bool ret = InitLogEx(config_dir.c_str(), kLogCategory.c_str(), log_full_path.c_str());
    if (ret)
        LOGI("Init success, config dir: [%s], log file: [%s]",
             config_dir.c_str(), log_full_path.c_str());

    return ret;
}

} // namespace util::log
