#pragma once

#include "json/json.h"

#include <sstream>
#include <fstream>


namespace util {
namespace json {

inline std::string JsonToString(const Json::Value &value) {
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ostringstream ss;
    writer->write(value, &ss);
    return ss.str();
}

inline bool ParseJsonString(std::string json_info, Json::Value &root, std::string *json_err) {
    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
    
    if (!reader->parse(json_info.c_str(), json_info.c_str() + json_info.length(), &root, json_err)) {
        return false;
    }
    
    return true;
}

inline bool ParseJsonFile(const std::string &file_path, Json::Value &root, std::string &json_err) {
    std::ifstream fs(file_path);
    if (!fs.is_open())
        return false;
    
    Json::CharReaderBuilder reader_builder;
    return Json::parseFromStream(reader_builder, fs, &root, &json_err);
}

inline void Dump(const Json::Value &value, std::fstream &f) {
    if (!f.is_open())
        return;
    
    std::string json_str = JsonToString(value);
    f.write(json_str.c_str(), json_str.size());
}

}   // namespace json
}   // namespace util
