//
// Created by Lee on 2022/1/10.
//

#ifndef LIB_IMAGEDUPLICATE_SDK_FILEFILTERUTIL_H
#define LIB_IMAGEDUPLICATE_SDK_FILEFILTERUTIL_H

#include <vector>
#include <string>
#include <set>

#include "FileUtil.h"
#include "StringUtil.h"

namespace util {
namespace file {

class Filter {
public:
    struct Config {
        unsigned int file_size_max;         // 文件尺寸最大值, 过滤大于该值的文件, 0为不做限制
        unsigned int file_size_min;         // 文件尺寸最小值, 过滤小与该值的文件, 0为不做限制
        const char **exclude_paths;         // 排除路径列表
        unsigned int exclude_paths_count;   // 配出路径列表的大小
        const char **include_exts;          // 不过滤的文件后缀列表, 需要带点号, 不区分大小写, 如(.jpg, .png)
        unsigned int include_exts_count;    // 不过滤文件后缀列表的大小
        bool is_filter_hidden;              // 是否过滤隐藏文件
    };
    
    Filter(Config config) :
            file_size_min_(config.file_size_min),
            file_size_max_(config.file_size_max),
            is_filter_hidden_(config.is_filter_hidden){
        if (nullptr != config.exclude_paths) {
            for (int i = 0; i < config.exclude_paths_count; ++i) {
                exclude_paths_.push_back(config.exclude_paths[i]);
            }
        }
        if (nullptr != config.include_exts) {
            for (int i = 0; i < config.include_exts_count; ++i) {
                include_exts_.insert(util::str::ToLower(config.include_exts[i]));
            }
        }
    }
    
    bool IsFilterDir(const std::string &u8path) {
        // 过滤文件路径
        for (const std::string &exclude_path: exclude_paths_) {
            if (std::string::npos != u8path.find(exclude_path)) {
                return true;
            }
        }
        
        return false;
    }
    
    bool IsFilterFile(const std::string &u8path) {
        // 过滤隐藏文件
        if (is_filter_hidden_ && IsHidden(u8path))
            return true;
    
        // 过滤文件路径
        for (const std::string &exclude_path: exclude_paths_) {
            
            if (std::string::npos != u8path.find(exclude_path)) {
                return true;
            }
        }
        
        // 过滤文件后缀
        std::string ext = util::str::ToLower(util::file::GetExtension(u8path));
        if (!include_exts_.empty()
            && 0 == include_exts_.count(ext))
            return true;
        
        // 过滤文件大小
        if (0 == file_size_min_ && 0 == file_size_max_)
            return false;
        uint64_t fz = util::file::getFileSize(u8path);
        // 大小为 0 则不做限制
        if ((0 != file_size_min_ && fz < file_size_min_)
            || 0 != file_size_max_ && fz > file_size_max_)
            return true;
        
        return false;
    }

private:
    unsigned int file_size_min_;
    unsigned int file_size_max_;
    std::vector<std::string> exclude_paths_;
    std::set<std::string> include_exts_;
    bool is_filter_hidden_;
};

}   // namespace file
}   // namespace util
#endif //LIB_IMAGEDUPLICATE_SDK_FILEFILTERUTIL_H
