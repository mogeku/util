#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "SingletonUtil.h"

#ifdef MY_DEBUG
using std::cout;
using std::endl;
#endif

namespace util {

class ThreadPool {
public:
    using PoolSeconds = std::chrono::seconds;

    /** 线程池的配置
     * core_threads: 核心线程个数，线程池中最少拥有的线程个数，初始化就会创建好的线程，常驻于线程池
     *
     * max_threads: >=core_threads，当任务的个数太多线程池执行不过来时，
     * 内部就会创建更多的线程用于执行更多的任务，内部线程数不会超过max_threads
     *
     * max_task_size: 内部允许存储的最大任务个数，暂时没有使用
     *
     * time_out: Cache线程的超时时间，Cache线程指的是max_threads-core_threads的线程,
     * 当time_out时间内没有执行任务，此线程就会被自动回收
     */
    struct ThreadPoolConfig {
        int core_threads;
        int max_threads;
        int max_task_size;
        PoolSeconds time_out;
    };

    /**
     * 线程的状态：有等待、运行、停止
     */
    enum class ThreadState {
        kInit = 0, kWaiting = 1, kRunning = 2, kStop = 3
    };

    /**
     * 线程的种类标识：标志该线程是核心线程还是Cache线程，Cache是内部为了执行更多任务临时创建出来的
     */
    enum class ThreadFlag {
        kInit = 0, kCore = 1, kCache = 2
    };

    using ThreadPtr = std::shared_ptr<std::thread>;
    using ThreadId = std::atomic<int>;
    using ThreadStateAtomic = std::atomic<ThreadState>;
    using ThreadFlagAtomic = std::atomic<ThreadFlag>;

    /**
     * 线程池中线程存在的基本单位，每个线程都有个自定义的ID，有线程种类标识和状态
     */
    struct ThreadWrapper {
        ThreadPtr ptr;
        ThreadId id;
        ThreadFlagAtomic flag;
        ThreadStateAtomic state;

        ThreadWrapper() {
            ptr = nullptr;
            id = 0;
            state.store(ThreadState::kInit);
        }
    };

    using ThreadWrapperPtr = std::shared_ptr<ThreadWrapper>;
    using ThreadPoolLock = std::unique_lock<std::mutex>;

    static ThreadPool &instance() { return SingleTon<ThreadPool>::instance(); }

    ThreadPool() {
        this->total_function_num_.store(0);
        this->waiting_thread_num_.store(0);

        this->thread_id_.store(0);
        this->is_shutdown_.store(false);
        this->is_shutdown_now_.store(false);

        is_available_.store(false);
    }
    ThreadPool(ThreadPoolConfig config) {
        Init(config);
    }

    ~ThreadPool() { ShutDown(); }

    bool Init(ThreadPoolConfig config) {
        this->total_function_num_.store(0);
        this->waiting_thread_num_.store(0);

        this->thread_id_.store(0);
        this->is_shutdown_.store(false);
        this->is_shutdown_now_.store(false);

        if (IsValidConfig(config)) {
            is_available_.store(true);
        } else {
            is_available_.store(false);
        }
        config_ = config;

        return IsAvailable();
    }

    bool Reset(ThreadPoolConfig config) {
        if (!IsValidConfig(config)) {
            return false;
        }
        if (config_.core_threads != config.core_threads) {
            return false;
        }
        config_ = config;
        return true;
    }

    // 开启线程池功能
    bool Start() {
        if (!IsAvailable()) {
            return false;
        }
        int core_thread_num = config_.core_threads;
#ifdef MY_DEBUG
        cout << "Init thread num " << core_thread_num << endl;
#endif
        while (core_thread_num-- > 0) {
            AddThread(GetNextThreadId());
        }
#ifdef MY_DEBUG
        cout << "Init thread end" << endl;
#endif
        return true;
    }

    // 获取正在处于等待状态的线程的个数
    int GetWaitingThreadSize() { return this->waiting_thread_num_.load(); }

    // 获取线程池中当前线程的总个数
    int GetTotalThreadSize() {
        ThreadPoolLock lock(worker_thread_mutex_);
        return this->worker_threads_.size();
    }

    // 放在线程池中执行函数
    template<typename F, typename... Args>
    auto Run(F &&f, Args &&... args) -> std::shared_ptr<std::future<std::result_of_t<F(Args...)>>> {
        if (this->is_shutdown_.load() || this->is_shutdown_now_.load() || !IsAvailable()) {
            return nullptr;
        }

        {
            // 防止多线程同时添加新线程, 导致新添加的线程刚进入等待状态, 还没有开始执行任务, 导致这里跳过创建新线程
            ThreadPoolLock lock(this->thread_add_mutex_);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (GetWaitingThreadSize() == 0 && GetTotalThreadSize() < config_.max_threads) {
                AddThread(GetNextThreadId(), ThreadFlag::kCache);
            }
        }

        using return_type = std::result_of_t<F(Args...)>;
        auto task = std::make_shared<std::packaged_task<return_type() >>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        total_function_num_++;

        std::future<return_type> res = task->get_future();
        {
            ThreadPoolLock lock(this->task_mutex_);
            this->tasks_.emplace([task]() { (*task)(); });
        }
        this->task_cv_.notify_one();
        return std::make_shared<std::future<std::result_of_t<F(Args...)>>>(std::move(res));
    }

    // 获取当前线程池已经执行过的函数个数
    int GetRunnedFuncNum() { return total_function_num_.load(); }

    // 关掉线程池，内部还没有执行的任务会继续执行
    void ShutDown() {
        ShutDown(false);
#ifdef MY_DEBUG
        cout << "shutdown" << endl;
#endif
    }

    // 执行关掉线程池，内部还没有执行的任务直接取消，不会再执行
    void ShutDownNow() {
        ShutDown(true);
#ifdef MY_DEBUG
        cout << "shutdown now" << endl;
#endif
    }

    // 当前线程池是否可用
    bool IsAvailable() { return is_available_.load(); }

private:
    void ShutDown(bool is_now) {
        if (is_available_.load()) {
            if (is_now) {
                this->is_shutdown_now_.store(true);
            } else {
                this->is_shutdown_.store(true);
            }
            this->task_cv_.notify_all();

            while (this->GetTotalThreadSize() != 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

            is_available_.store(false);
        }
    }

    void AddThread(int id) { AddThread(id, ThreadFlag::kCore); }

    void AddThread(int id, ThreadFlag thread_flag) {
#ifdef MY_DEBUG
        cout << "AddThread " << id << " flag " << static_cast<int>(thread_flag) << endl;
#endif
        ThreadWrapperPtr thread_ptr = std::make_shared<ThreadWrapper>();
        thread_ptr->id.store(id);
        thread_ptr->flag.store(thread_flag);
        auto func = [this, thread_ptr]() {
            for (;;) {
                std::function<void()> task;
                {
                    ThreadPoolLock lock(this->task_mutex_);
                    if (thread_ptr->state.load() == ThreadState::kStop) {
                        break;
                    }
#ifdef MY_DEBUG
                    cout << "thread id " << thread_ptr->id.load() << " running start" << endl;
#endif
                    thread_ptr->state.store(ThreadState::kWaiting);
                    ++this->waiting_thread_num_;
                    bool is_timeout = false;
                    if (thread_ptr->flag.load() == ThreadFlag::kCore) {
                        this->task_cv_.wait(lock, [this, thread_ptr] {
                            return (this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                                    thread_ptr->state.load() == ThreadState::kStop);
                        });
                    } else {
                        this->task_cv_.wait_for(lock, this->config_.time_out, [this, thread_ptr] {
                            return (this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                                    thread_ptr->state.load() == ThreadState::kStop);
                        });
                        is_timeout = !(this->is_shutdown_ || this->is_shutdown_now_ || !this->tasks_.empty() ||
                                       thread_ptr->state.load() == ThreadState::kStop);
                    }
                    --this->waiting_thread_num_;
#ifdef MY_DEBUG
                    cout << "thread id " << thread_ptr->id.load() << " running wait end" << endl;
#endif

                    if (is_timeout) {
                        thread_ptr->state.store(ThreadState::kStop);
                    }

                    if (thread_ptr->state.load() == ThreadState::kStop) {
#ifdef MY_DEBUG
                        cout << "thread id " << thread_ptr->id.load() << " state stop" << endl;
#endif
                        break;
                    }
                    if (this->is_shutdown_ && this->tasks_.empty()) {
#ifdef MY_DEBUG
                        cout << "thread id " << thread_ptr->id.load() << " shutdown" << endl;
#endif
                        break;
                    }
                    if (this->is_shutdown_now_) {
#ifdef MY_DEBUG
                        cout << "thread id " << thread_ptr->id.load() << " shutdown now" << endl;
#endif
                        break;
                    }
                    thread_ptr->state.store(ThreadState::kRunning);
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                task();
            }

            {
                // 将结束的线程从线程链表中删除
                ThreadPoolLock lock(this->worker_thread_mutex_);
                this->worker_threads_.remove(thread_ptr);
            }
#ifdef MY_DEBUG
            cout << "thread id " << thread_ptr->id.load() << " running end" << endl;
#endif
        };
        thread_ptr->ptr = std::make_shared<std::thread>(std::move(func));
        if (thread_ptr->ptr->joinable()) {
            thread_ptr->ptr->detach();
        }
        {
            ThreadPoolLock lock(this->worker_thread_mutex_);
            this->worker_threads_.emplace_back(std::move(thread_ptr));
        }
    }

    void Resize(int thread_num) {
        if (thread_num < config_.core_threads) return;
        int old_thread_num = worker_threads_.size();
#ifdef MY_DEBUG
        cout << "old num " << old_thread_num << " resize " << thread_num << endl;
#endif
        if (thread_num > old_thread_num) {
            while (thread_num-- > old_thread_num) {
                AddThread(GetNextThreadId());
            }
        } else {
            int diff = old_thread_num - thread_num;
            auto iter = worker_threads_.begin();
            while (iter != worker_threads_.end()) {
                if (diff == 0) {
                    break;
                }
                auto thread_ptr = *iter;
                if (thread_ptr->flag.load() == ThreadFlag::kCache &&
                    thread_ptr->state.load() == ThreadState::kWaiting) {  // wait
                    thread_ptr->state.store(ThreadState::kStop);          // stop;
                    --diff;
                    iter = worker_threads_.erase(iter);
                } else {
                    ++iter;
                }
            }
            this->task_cv_.notify_all();
        }
    }

    int GetNextThreadId() { return this->thread_id_++; }

    bool IsValidConfig(ThreadPoolConfig config) {
        if (config.core_threads < 1 || config.max_threads < config.core_threads || config.time_out.count() < 1) {
            return false;
        }
        return true;
    }

private:
    ThreadPoolConfig config_;

    std::list<ThreadWrapperPtr> worker_threads_;
    std::mutex thread_add_mutex_;
    std::mutex worker_thread_mutex_;

    std::queue<std::function<void()>> tasks_;
    std::mutex task_mutex_;
    std::condition_variable task_cv_;

    std::atomic<int> total_function_num_;
    std::atomic<int> waiting_thread_num_;
    std::atomic<int> thread_id_;

    std::atomic<bool> is_shutdown_now_;
    std::atomic<bool> is_shutdown_;
    std::atomic<bool> is_available_;
};


}  // namespace util

#endif
