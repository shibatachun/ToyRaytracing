#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>

class ThreadPool {
public:
    // 构造函数，创建指定数量的工作线程
    explicit ThreadPool(size_t threadCount) : stop(false) {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;

                    {   // 获取任务队列的锁
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        // 等待任务或停止信号
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                            });
                        // 如果停止且任务队列为空，退出线程
                        if (this->stop && this->tasks.empty())
                            return;
                        // 取出任务
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                        ;
                    }

                    // 执行任务
                    task();
                }
                });
        }
    }

    // 添加新任务到线程池
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using returnType = typename std::invoke_result<F, Args...>::type;

        // 创建一个可调用的任务
        auto task = std::make_shared<std::packaged_task<returnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        // 获取任务的 future 对象
        std::future<returnType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // 如果线程池已停止，抛出异常
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            // 将任务添加到队列
            tasks.emplace([task]() { (*task)(); });
        }
        // 通知一个等待中的线程
        condition.notify_one();
        return res;
    }

    // 析构函数，等待所有线程完成
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

private:
    // 工作线程集合
    std::vector<std::thread> workers;
    // 任务队列
    std::queue<std::function<void()>> tasks;

    // 同步机制
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};