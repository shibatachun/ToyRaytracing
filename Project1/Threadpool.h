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
    // ���캯��������ָ�������Ĺ����߳�
    explicit ThreadPool(size_t threadCount) : stop(false) {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;

                    {   // ��ȡ������е���
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        // �ȴ������ֹͣ�ź�
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                            });
                        // ���ֹͣ���������Ϊ�գ��˳��߳�
                        if (this->stop && this->tasks.empty())
                            return;
                        // ȡ������
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                        ;
                    }

                    // ִ������
                    task();
                }
                });
        }
    }

    // ����������̳߳�
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using returnType = typename std::invoke_result<F, Args...>::type;

        // ����һ���ɵ��õ�����
        auto task = std::make_shared<std::packaged_task<returnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        // ��ȡ����� future ����
        std::future<returnType> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            // ����̳߳���ֹͣ���׳��쳣
            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");
            // ��������ӵ�����
            tasks.emplace([task]() { (*task)(); });
        }
        // ֪ͨһ���ȴ��е��߳�
        condition.notify_one();
        return res;
    }

    // �����������ȴ������߳����
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
    // �����̼߳���
    std::vector<std::thread> workers;
    // �������
    std::queue<std::function<void()>> tasks;

    // ͬ������
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
};