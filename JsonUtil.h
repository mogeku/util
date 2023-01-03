#pragma once

#include <sstream>
#include <fstream>

#include "util/nlohmann/json.hpp"


namespace util {
namespace json {

using json = nlohmann::json;

inline json ParseJsonFile(const std::string &file_path) {
    std::ifstream fs(file_path);
    if (!fs.is_open())
        return false;

    json data =json::parse(fs);
    return data;
}
inline json ParseJsonFile(const char* file_path) {
    return ParseJsonFile(std::string(file_path));
}

}   // namespace json
}   // namespace util
