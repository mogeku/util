//
// Created by Lee on 2022/1/13.
//

#ifndef LIB_IMAGEDUPLICATE_SDK_SINGLETONUTIL_H
#define LIB_IMAGEDUPLICATE_SDK_SINGLETONUTIL_H

#include <iostream>
#include <mutex>
#include <thread>

namespace util {

template<typename T>
class SingleTon {
public:
    static T &instance() {
        std::call_once(once_, &SingleTon::init);
        return *value_;
    }

private:
    SingleTon();
    
    ~SingleTon();
    
    SingleTon(const SingleTon &) = delete;
    
    SingleTon &operator=(const SingleTon &) = delete;
    
    static void init() { value_ = std::make_shared<T>(); }

private:
    static std::shared_ptr<T> value_;
    
    static std::once_flag once_;
};

template<typename T>
std::shared_ptr<T> SingleTon<T>::value_ = nullptr;

template<typename T>
std::once_flag SingleTon<T>::once_;

}  // namespace util


#endif //LIB_IMAGEDUPLICATE_SDK_SINGLETONUTIL_H
