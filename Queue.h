//
// Created by Lee on 2022/2/25.
//

#ifndef LIB_IMAGEDUPLICATE_SDK_QUEUE_H
#define LIB_IMAGEDUPLICATE_SDK_QUEUE_H

#include <queue>
#include <mutex>
#include <string>

namespace util {

template<typename T>
class Queue {
public:
    Queue() : is_no_more_(false) {}
    
    void push(const T &item) {
        std::lock_guard<std::mutex> lk(mutex_);
        queue_.push(std::make_shared<T>(item));
        cv_.notify_one();
    }
    
    std::shared_ptr<T> pop() {
        std::unique_lock<std::mutex> lk(mutex_);
        
        cv_.wait(lk, [this]() {
            return (!queue_.empty()) || (is_no_more_ && queue_.empty());
        });
        
        if (queue_.empty())
            return nullptr;
        
        std::shared_ptr<T> tmp = queue_.front();
        queue_.pop();
        return tmp;
    }
    
    void clear() {
        is_no_more_ = false;
        std::lock_guard<std::mutex> lk(mutex_);
        std::queue<std::shared_ptr<T>> empty;
        queue_.swap(empty);
    }
    
    bool empty() {
        return queue_.empty();
    }
    
    void SetNoMoreFlag() {
        is_no_more_ = true;
        cv_.notify_all();
    }
    
    bool IsNoMore() const { return is_no_more_; }

private:
    std::queue<std::shared_ptr<T>> queue_;
    
    std::mutex mutex_;
    std::condition_variable cv_;
    bool is_no_more_;
};

}  // namespace util

#endif //LIB_IMAGEDUPLICATE_SDK_QUEUE_H
