#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class thread_pool
{
public:
    explicit thread_pool(int thread_count = 0);
    ~thread_pool();

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    thread_pool(thread_pool&&) = delete;
    thread_pool& operator=(thread_pool&&) = delete;

    template <typename Func, typename ... Args>
    auto schedule(Func&& func, Args&& ... args);

private:
    std::vector<std::thread> m_threads;
    std::queue<std::packaged_task<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    bool m_stop{false};
};

thread_pool::thread_pool(int thread_count)
{
    auto routine = [this] () {
        while (true) {
            std::packaged_task<void()> tsk;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                while (!m_stop && m_tasks.empty()) {
                    m_condition.wait(lock);
                }
                if (m_stop && m_tasks.empty()) {
                    return;
                }
                tsk = std::move(m_tasks.front());
                m_tasks.pop();
            }
            tsk();
        }
    };
    if (thread_count == 0) {
        thread_count = std::thread::hardware_concurrency();
    }
    for (int i = 0; i < thread_count; ++i) {
        m_threads.emplace_back(routine);
    }
}

thread_pool::~thread_pool()
{
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
    }

    m_condition.notify_all();
    for (auto& thread : m_threads) {
        thread.join();
    }
}

template <typename Func, typename ... Args>
auto thread_pool::schedule(Func&& func, Args&& ... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_stop) { 
        throw std::runtime_error("Schedule on destroyed scheduler.");
    }
    using ResultT = std::invoke_result_t<Func, Args ...>;
    std::packaged_task<ResultT()> packaged_task{std::bind(std::forward<Func>(func), std::forward<Args>(args)...)};
    auto future = packaged_task.get_future();
    m_tasks.emplace(std::move(packaged_task));
    m_condition.notify_one();
    return future;
}
