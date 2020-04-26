#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <numeric>

#include "thread_pool.h"

using namespace std::chrono_literals;
using Duration = std::chrono::duration<int>;

void sleep_for_one_sec()
{
    std::this_thread::sleep_for(Duration{1s});
}

void foo1(int value)
{
    std::cout << "Started thread: " << value << '\n';
    sleep_for_one_sec();
    std::cout << "\t\t\tFinished Thread: " << value << '\n';
}

int foo2(int value)
{
    std::cout << "Started thread: " << value << '\n';
    sleep_for_one_sec();
    std::cout << "\t\t\tFinished thread: " << value << '\n';
    return value;
}

void test(const int pool_capacity, const int task_count)
{
    std::cout << "\n======================================\n";

    thread_pool pool(pool_capacity);
    std::vector<std::future<int>> results;
    int expected_result = 0;

    for (int i = 1; i <= task_count; ++i) {
        pool.schedule(foo1, i);
        results.emplace_back(pool.schedule([](int v) { return v; }, i));
        results.emplace_back(pool.schedule(foo2, i));
        expected_result += i * 2;
    }

    const auto actual_result = std::accumulate(begin(results), end(results), 0, [] (const auto sum, auto& fut) { return sum + fut.get(); });
    if (expected_result != actual_result) {
        std::cout << "Failed: " << actual_result << " != " << expected_result << "\n";
    }
}

int main()
{
    const int thread_count = 4;
    const int task_count = 10;
    const auto start_time = std::chrono::system_clock::now();
    test(thread_count, task_count);
    const auto end_time = std::chrono::system_clock::now();
    std::cout << "\nWorked for " << std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count() << " seconds to perform " << task_count * 3 << " tasks.\n";
}
