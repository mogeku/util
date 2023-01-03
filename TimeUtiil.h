#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>

namespace util {
namespace time {

inline std::string GetCurrentTime(const std::string format="%Y-%m-%d %H:%M:%S") {
  // 获取当前时间
  auto currentTime = std::chrono::system_clock::now();

  // 将时间转换为时间点
  auto timePoint = std::chrono::time_point_cast<std::chrono::seconds>(currentTime);

  // 获取从 1970 年 1 月 1 日起经过的秒数
  auto seconds = timePoint.time_since_epoch().count();

  // 将秒数转换为本地时间
  tm* localTime = localtime(&seconds);

  // 格式化时间
  char formattedTime[100];
  strftime(formattedTime, sizeof(formattedTime), format.c_str(), localTime);

  return std::string(formattedTime);
}

}   // namespace time
}   // namespace util
