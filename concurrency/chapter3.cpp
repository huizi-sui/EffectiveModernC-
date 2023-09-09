#include <iostream>
#include <thread>

void thread_task() {
    std::cout << "hello thread" << std::endl;
}

int main() {
    std::thread t(thread_task);
    t.join();

    return 0;
}