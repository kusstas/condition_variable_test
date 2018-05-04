#include <iostream>
#include <iomanip>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <vector>

int const ID_FIRST_THREAD = 0;
int const COUNT_THREAD    = 40;
int const MS_TIME_SLEEP   = 100;
int const COUNT_COMPUTES  = 10;

std::mutex m;
std::condition_variable condVar;
int idCurrentThread; // id of current thread that push to queue
bool isRun = false;

void compute(std::queue<int>& result, int id);

int main()
{
    std::vector<std::thread> workers;
    std::queue<int> q;  

    for (int i = COUNT_THREAD - 1; i >= 0; i--) { // create in revevrce sequence (ony for test)
        workers.emplace_back(compute, std::ref(q), i);
    }

    for (auto& w : workers) { // wait all threads
        w.join();
    }

    int i = 0;
    while (!q.empty()) { // print queue
        std::cout << std::setw(2) << q.front() << " ";
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
    if (id == ID_FIRST_THREAD) {
        idCurrentThread = ID_FIRST_THREAD;
        isRun = true;
    }

    { // scope for create unique_lock
        std::unique_lock<std::mutex> lock(m);
        condVar.wait(lock, [] () { return isRun; }); // wait first thread
    }
    
    
    for (int i = 0; i < COUNT_COMPUTES; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MS_TIME_SLEEP)); // fake compute

        std::unique_lock<std::mutex> lock(m);
        condVar.wait(lock, [id] () { return id == idCurrentThread; } ); // wait previos thread
        result.push(id); // push id of thread
        idCurrentThread = (idCurrentThread < COUNT_THREAD - 1) ? idCurrentThread + 1 : 0;
        condVar.notify_all();
    }
}