#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <stdexcept>
/*
std::mutex介绍
std::mutex是C++11中最基本的互斥量，mutex对象提供了独占所有权的特性——即不支持递归地对mutex
对象上锁，而std::recursive_lock则可以递归地对互斥量对象上锁。

成员函数介绍：
1. 构造函数，std::mutex不允许拷贝构造，也不允许移动构造，最初产生地mutex对象是处于unlocked状态的。
2. lock(), 调用线程将锁住该互斥量。线程调用该函数会发生下面3种情况：
(1) 如果该互斥量当前没有被锁住，则调用线程将该互斥量锁住，直到调用unlock之前，该线程一直拥有该锁。
(2) 如果当前互斥量被其他线程锁住，则当前地调用线程被阻塞住。
(3) 如果当前互斥量被当前调用线程锁住，则会产生死锁（deadlock）。
3. unlock(): 解锁，释放对互斥量地所有权
4. try_lock()：尝试锁住互斥量，如果互斥量被其他线程占有，则当前线程也不会被阻塞。
线程调用该函数会出现下面3种情况：
(1) 如果当前互斥量没有被其他线程占有，则该线程锁住互斥量，直到该线程调用unlock释放互斥量
(2) 如果当前互斥量被其他线程锁住，则当前调用线程返回false，而并不会被阻塞掉
(3) 如果当前互斥量被当前调用线程锁住，则会产生死锁（deadlock）。
示例：
*/
volatile int counter(0); // non-atomic counter
std::mutex mtx; // locks access to counter

void attempt_10k_increase() {
    for(int i = 0; i < 10000; ++i) {
        if(mtx.try_lock()) { // only increase if currently not locked
            ++counter;
            mtx.unlock();
        }
    }
}
void test1() {
    std::thread threads[10];
    for(int i = 0; i < 10; ++i) {
        threads[i] = std::thread(attempt_10k_increase);
    }
    for(auto& th : threads) {
        th.join();
    }
    std::cout << counter << " successful increases of the counter.\n";
}
/*
std::recursive_mutex介绍
std::recursive_mutex与std::mutex一样，也是一种可以被上锁的对象，但是和std::mutex
不同的是，std::recursive_mutex允许同一个线程对互斥量多次上锁（即递归上锁），
来获得对互斥量对象的多层所有权，std::recursive_mutex释放互斥量时需要调用与该锁层次
深度相同次数的unlock(), 可理解为lock()次数与unlock()次数相同，除此之外，
std::recursive_mutex的特性与std::mutex大致相同
*/
class Counter {
public:
    Counter() : count(0) {}
    int add(int val) {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        count += val;
        return count;
    }
    int increment() {
        std::lock_guard<std::recursive_mutex> lock(mutex);
        return add(1);
    }
private:
    std::recursive_mutex mutex;
    int count;
};

Counter c;

void change_count(void*) {
    std::cout << "count == " << c.increment() << std::endl;
}
void test2() {
    std::thread threads[10];
    for(int i = 0; i < 10; ++i) {
        threads[i] = std::thread(change_count, nullptr);
    }
    for(auto& th : threads) {
        th.join();
    }
}
/*
std::time_mutex介绍
std::time_mutex比std::mutex多了两个成员函数，try_lock_for(), try_lock_until()

try_lock_for函数接受一个时间范围，表示在这一段时间范围之内线程如果没有获得锁则被阻塞住（
与std::mutex的try_lock()不同，try_lock如果被调用时没有获得锁则直接返回false），
如果在此期间其他线程释放了锁，则该线程可以获得对互斥量的锁，如果超时（即在指定时间内
还是没有获得锁）， 则返回false。

try_lock_until函数则接受一个时间点作为参数，在指定时间点未到来之前线程如果没有获得锁则被阻塞住，
如果在此期间其他线程释放了锁，则该线程可以获得对互斥量的锁，如果超时(即在指定时间内还是没有获得锁)，
则返回false
*/
std::timed_mutex tmtx;
void fireworks() {
    // waiting to get a lock: each thread prints "-" every 200ms
    while(!tmtx.try_lock_for(std::chrono::milliseconds(200))) {
        std::cout << "-";
    }
    // got a lock! - wait for 1s, then this thread prints "*"
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "*\n";
    tmtx.unlock();
}
void test3() {
    std::thread threads[10];
    for(int i = 0; i < 10; ++i) {
        threads[i] = std::thread(fireworks);
    }
    for(auto& th : threads) {
        th.join();
    }

}
/*
std::recursive_timed_mutex和std::recursive_mutex与std::mutex的关系一样，
std::recursive_timed_mutex的特性也可以从std::timed_mutex推导出来。
*/

/*
std::lock_guard介绍
与Mutex RAII相关，方便线程对互斥量上锁
*/
void print_even(int x) {
    if(x % 2 == 0) {
        std::cout << x << " is even\n";
    } else {
        throw(std::logic_error("not even"));
    }
}
void print_thread_id(int id) {
    try {
        std::lock_guard<std::mutex> lock(mtx);
        print_even(id);
    } catch(std::logic_error&) {
        std::cout << "[exception caught]\n";
    }
}
void test4() {
    std::thread threads[10];
    for(int i = 0; i < 10; ++i) {
        threads[i] = std::thread(print_thread_id, i + 1);
    }
    for(auto& th : threads) {
        th.join();
    }
}
/*
std::uniqe_lock介绍
与Mutex RAII相关，方便线程对互斥量上锁，但提供了更好的上锁和解锁控制
*/
void print_block(int n, char c) {
    std::unique_lock<std::mutex> lck(mtx);
    for(int i = 0; i < n; ++i) {
        std::cout << c;
    }
    std::cout << "\n";
}
void test5() {
    std::thread th1(print_block, 50, '*');
    std::thread th2(print_block, 50, '$');
    th1.join();
    th2.join();
}

int main() {
    // test1();
    // test2();
    // test3();
    // test4();
    test5();
}