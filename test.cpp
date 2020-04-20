#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <numeric>

#include "thread_pool.h"

using namespace std::chrono_literals;
using Duration = std::chrono::duration<int>;

int work_for(const Duration dur, int value)
{
    std::cout << "Started thread: " << value << '\n';
    std::this_thread::sleep_for(dur);
    std::cout << "\t\t\tFinished thread: " << value << '\n';
    return value;
}

int test(const int task_count, const Duration task_duration, const int pool_capacity)
{
    std::cout << "\n======================================\n";
    const auto start_time = std::chrono::system_clock::now();
    std::vector<std::future<int>> results;
    {
        thread_pool pool(pool_capacity);
        for (int i = 0; i < task_count; ++i) {
            results.emplace_back(pool.schedule(work_for, task_duration, i));
        }
    }
    const auto end_time = std::chrono::system_clock::now();
    std::cout << "\nWorked for " << std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count() << " seconds to perform " << task_count << " one-second tasks.\n";
    return (9 * 10 / 2) == std::accumulate(begin(results), end(results), 0, [] (const auto sum, auto& fut) { return sum + fut.get(); });
}

int main()
{
    if (!test(10, 1s, 3)) {
        std::cout << "FAILED\n";
    }
}
