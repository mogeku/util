//
// Created by Lee on 2022/1/10.
//

#ifndef LIB_IMAGEDUPLICATE_SDK_PROGRESSCONTROLUTIL_H
#define LIB_IMAGEDUPLICATE_SDK_PROGRESSCONTROLUTIL_H

#include "lib_ImageDuplicate_SDK.h"

#include <string>
#include <chrono>
#include <atomic>

#include "util/JsonUtil.h"
#include "util/SingletonUtil.h"
#include "util/LogUtil.h"

#define CHECK_IS_STOP_RETURN(ret) \
if (util::ProgressControl::instance().IsStop()) { \
    return ret; \
}
#define CHECK_IS_STOP_BREAK() \
if (util::ProgressControl::instance().IsStop()) { \
    break; \
}

namespace util {

using namespace image_duplicate;

class ProgressControl {
public:
    using Seconds = std::chrono::seconds;
    using Milliseconds = std::chrono::milliseconds;
public:
    ProgressControl() :
            progress_cb_(nullptr),
            progress_total_(kDefaultTotal),
            current_(0),
            status_(kProgressRuning) {}
    
    static ProgressControl &instance() { return SingleTon<ProgressControl>::instance(); }
    
    void SetTotal(double total) { progress_total_ = max(0.0, total); }
    
    double GetTotal() const { return progress_total_.load(); }
    
    void SetCallback(ProgressCallback progress_cb) { progress_cb_ = progress_cb; }
    
    bool IsStop() { return kProgressStop == status_; }
    
    bool IsRunning() { return kProgressRuning == status_; }
    
    void ProgressBegin() {
        progress_cb_ = nullptr;
        progress_total_ = kDefaultTotal;
        
        current_ = 0;
        status_ = kProgressRuning;
    }
    
    void Callback(Json::Value extra_info = Json::Value(), double step = kDefaultStep) {
        if (kProgressRuning == status_) current_ = current_ + step;
        std::string progress_info_str = _ToJsonStr(extra_info);
        if (progress_cb_) {
            try {
                std::unique_lock<std::mutex> lk(lock_);
                progress_cb_(status_, {progress_info_str.c_str(), progress_info_str.size()});
            } catch (std::exception &e) {
                LOGW("Progress callback function crashed, error message {%s}", e.what());
            }
        }
    }
    
    void ProgressEnd(Errors return_code) {
        status_ = kProgressEnd;
        Json::Value extra_info;
        extra_info["return_code"] = return_code;
        extra_info["return_code_msg"] = kErrorCodeMsg.at(return_code);
        Callback(extra_info);
    }
    
    Milliseconds ProgressStop(const Seconds time_out = kDefaultTimeout) {
        if (!IsRunning())
            return Milliseconds(0);
        status_ = kProgressStop;
        return _WaitForStopFin(time_out);
    }

private:
    Milliseconds _WaitForStopFin(const Seconds time_out) {
        static const auto kSleepTime = Milliseconds(500);
        auto past_time = Milliseconds(0);
        while (IsStop() && past_time <= time_out) {
            std::this_thread::sleep_for(kSleepTime);
            past_time += kSleepTime;
        }
        return past_time;
    }
    
    std::string _ToJsonStr(
            const Json::Value &extra_info,
            ProgressType progress_type = kProgressRuning
    ) const {
        Json::Value v;
        v["total"] = progress_total_.load();
        v["current"] = current_.load();
        v["ratio"] = current_ / progress_total_;
        if (!extra_info.empty()) v["extra_info"] = extra_info;
        return util::json::JsonToString(v);
    }

private:
    
    constexpr static const double kDefaultStep = 1.0;   // 1.0
    constexpr static double kDefaultTotal = 20.0 * 10000.0;  // 10w
    constexpr static const Seconds kDefaultTimeout = Seconds(10);  // 10s
    
    ProgressCallback progress_cb_;
    
    std::atomic<double> progress_total_;
    std::atomic<double> current_;
    
    ProgressType status_;
    
    std::mutex lock_;
};

}   // namespace util
#endif //LIB_IMAGEDUPLICATE_SDK_PROGRESSCONTROLUTIL_H