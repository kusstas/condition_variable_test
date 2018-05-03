#include <iostream>
#include <iomanip>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <vector>

int const COUNT_THREAD   = 40;
int const MS_TIME_SLEEP  = 100;
int const COUNT_COMPUTES = 10;

std::mutex m;
std::condition_variable cond_var;
int current_thread = 1;

void compute(std::queue<int>& result, int id);

int main()
{
    std::vector<std::thread> workers;
    std::queue<int> q;
    

    for (int i = 0; i < COUNT_THREAD; i++) {
        workers.emplace_back(compute, std::ref(q), i + 1);
    }

    cond_var.notify_one();

    for (auto& w : workers) {
        w.join();
    }

    int i = 0;
    while (!q.empty()) {
        std::cout << std::setw(3) << q.front() << " ";
        q.pop();
        i++;
        
        if (i == COUNT_THREAD) {
            std::cout << std::endl;
            i = 0;
        }      
    }

    return 0;
}

void compute(std::queue<int>& result, int id)
{
    for (int i = 0; i < COUNT_COMPUTES; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MS_TIME_SLEEP));

        std::unique_lock<std::mutex> lock(m);
        cond_var.wait(lock, [id] () { return id == current_thread; });
        result.push(id);
        current_thread = (current_thread == COUNT_THREAD) ? 1 : current_thread + 1;
        cond_var.notify_all();
    }
}