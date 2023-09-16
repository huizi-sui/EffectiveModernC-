#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <sstream> // std::stringstream
/*
前面介绍了多线程、互斥量、条件变量和异步编程相关的API。
为了性能和效率，需要开发一些lock-free的算法和数据结果，则需要深入理解。
这里介绍atomic头文件中的类和相关函数。

这个文件介绍atomic头文件中最简单的原子类型： atomic_flag。
atomic_flag是一种简单的原子布尔类型，只支持两种操作，test_and_set和clear。
*/
/*
std::atomic_flag构造函数
1. atomic_flag() noexcept = default;
2. atomic_flag(const atomic_flag& T) = delete;
只有默认构造函数，拷贝构造函数已经被禁用。
另外atomic_flag不能被拷贝，也不能被移动赋值

如果在初始化时没有明确使用ATOMIC_FLAG_INIT初始化，那么新创建的std::atomic_flag
对象的状态是未指定的(unspecified)(即没有被set也没有被clear)
ATOMIC_FLAG_INIT: 如果某个std::atomic_flag对象使用该宏初始化，那么可以保证
该std::atomic_flag对象在创建时处于clear状态。
*/
std::atomic<bool> ready(false); // can be checked without being set
std::atomic_flag winner = ATOMIC_FLAG_INIT; // always set when checked

void count1m(int id) {
    while(!ready) {
        // 若ready为false，则阻塞该线程
        std::this_thread::yield();
    }
    for(int i = 0; i < 1000000; ++i) {
        // 计数
    }
    // 若某个线程率先完成上面循环，则输出自己的ID
    // 此后其他线程执行test_and_set时if语句判断为false
    // 因此不会输出自身ID
    if(!winner.test_and_set()) {
        std::cout << "thread #" << id << " won!\n";
    }
}
void test1() {
    std::vector<std::thread> threads;
    std::cout << "spawning 10 threads that count to 1 million...\n";
    for(int i = 0; i < 10; ++i) {
        threads.push_back(std::thread(count1m, i));
    }
    ready = true;
    for(auto& th : threads) {
        th.join();
    }
}
/*
std::test_and_set函数原型如下：
bool test_and_set(memory_order sync = memory_order_seq_cst) volatile noexcept;
bool test_and_set(memory_order sync = memory_order_seq_cst) noexcept;

test_and_set()函数检查std::atmoic_flag标志，如果std::atmoic_flag之前没有被设置过，
则设置std::atmoic_flag的标志，并返回先前该std::atmoic_flag是否被设置过，如果之前
std::atmoic_flag对象已被设置，则返回true，否则返回false。

test_and_set操作是原子的(因此test_and_set是原子read_modify_write(RMW)操作)
test_and_set可以指定Memory Order。取值如下：
Memory Order值  Memory Order类型
memory_order_relaxed    Relaxed
memory_order_consume    Comsume
memory_order_acquire    Acquire
memory_order_release    Release
memory_order_acq_rel    Acquire/Release
memory_order_seq_cst    Sequentially consistent
*/
std::atomic_flag lock_stream = ATOMIC_FLAG_INIT;
std::stringstream stream;
void append_number(int x) {
    while(lock_stream.test_and_set()) {
        // 若设置了，则为true，阻塞
    }
    stream << "thread #" << x << '\n';
    lock_stream.clear();
}
void test2() {
    std::vector<std::thread> threads;
    for(int i = 1; i <= 10; ++i) {
        threads.push_back(std::thread(append_number, i));
    }
    for(auto& th : threads) {
        th.join();
    }
    std::cout << stream.str() << std::endl;
}
/*
clear()介绍
清除std::atomic_flag对象的标志位，即设置atomic_flag的值为false。clear函数原型如下：
void clear(memory_order sync = memory_order_seq_cst) volatile noexcept;
void clear(memory_order sync = memory_order_seq_cst) noexcept;

结合std::atomic_flag::test_and_set()和std::atomic_flag::clear()，std::atomic_flag
对象可以当作一个简单的自旋锁使用
*/
std::atomic_flag lock = ATOMIC_FLAG_INIT;
void f(int n) {
    for(int cnt = 0; cnt < 100; ++cnt) {
        /*
        若lock.test_and_set返回false，则表示上锁成功，其他线程调用lock.test_and_set返回true，
        进入自旋锁
        */
        while(lock.test_and_set(std::memory_order_acquire)) { // acquire lock
            // spin
        }
        std::cout << "Output from thread " << n << '\n';
        lock.clear(std::memory_order_release); // release lock
    }
}
void test3() {
    std::vector<std::thread> threads;
    for(int i = 1; i <= 10; ++i) {
        threads.emplace_back(f, i);
    }
    for(auto& th : threads) {
        th.join();
    }
    std::cout << stream.str() << std::endl;
}
int main() {
    // test1();
    // test2();
    test3();
}