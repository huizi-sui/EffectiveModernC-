#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <pthread.h>
#include <mutex>
/*
std::thread详解

std::thread构造函数
1. 默认构造函数
thread noexcept;
创建一个空的std::thread执行对象
2. 初始化构造函数
template<class Fn, class... Args>
explicit thread(Fn&& fn, Args...&& args);
创建一个std::thread对象，该对象可被joinable，新产生的线程会调用fn函数，
该函数的参数由args给出。
3. 拷贝构造函数
thread(const thread&) = delete;
意味着std::thread对象不可拷贝构造
4. 移动构造函数
thread(thread&& x) noexcept;
调用成功之后x不代表任何std::thread执行对象。

可被joinable的std::thread对象必须在他们销毁之前
被主线程join或者将其设置为detached
*/

void f1(int n) {
    for(int i = 0; i < 5; ++i) {
        std::cout << "Thread " << n << " executing.\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
void f2(int& n) {
    for(int i = 0; i < 5; ++i) {
        std::cout << "Thread 2 executing.\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
/*
std::thread赋值操作
1. 移动赋值操作
thread& operator=(thread&& rhs) noexcept;
如果当前对象不可joinable，需要传递一个右值引用（rhs）给move赋值操作；
如果当前对象可被joinable，则会调用terminate()报错
2. 拷贝赋值操作
thread& operator=(const thread&) = delete;
被禁用，因此std::thread对象不可拷贝赋值。
*/
void thread_task(int n ) {
    std::this_thread::sleep_for(std::chrono::seconds(n));
    std::cout << "hello thread " << std::this_thread::get_id()
              << " paused " << n << " seconds" << std::endl;
}

class Widget {
public:
    Widget() {
        std::cout << "Widget()" << std::endl;
    }
    // 拷贝构造函数
    Widget(const Widget& w) {
        std::cout << "Widget(const Widget&)" << std::endl;
    }
    // 移动构造函数
    Widget(Widget&& rhs) {
        std::cout << "Widget(Widget&& rhs)" << std::endl;
    }
    // 拷贝赋值函数
    Widget& operator=(const Widget& w) {
        std::cout << "Widget& operator=(const Widget& w)" << std::endl;
        return *this;
    }
    // 移动赋值函数
    Widget& operator=(Widget&& w) {
        std::cout << "Widget& operator=(Widget&& w)" << std::endl;
        return *this;
    }
};

/*
get_id: 获取线程ID，返回一个类型为std:thread::id的对象
*/
void foo() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void independentThread() {
    std::cout << "Starting concurrent thread.\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Exiting concurrent thread.\n";
}

void threadCaller() {
    std::cout << "Starting thread caller.\n";
    std::thread t(independentThread);
    t.detach();
    std::cout << t.get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Exiting thread caller.\n";
}

std::mutex iomutex;
void f(int num) {
    std::this_thread::sleep_for(std::chrono::seconds(1));

    sched_param sch;
    int policy;
    pthread_getschedparam(pthread_self(), &policy, &sch);
    std::lock_guard<std::mutex> lk(iomutex);
    std::cout << "Thread " << num << " is executing at priority " 
            << sch.sched_priority << std::endl;    
}

std::mutex g_display_mutex;
void foo1() {
    std::thread::id this_id = std::this_thread::get_id();

    g_display_mutex.lock();
    std::cout << "thread " << this_id << " sleeping...\n";
    g_display_mutex.unlock();

    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// "busy sleep" while suggesting that other threads run for a small amount of time
void little_sleep(std::chrono::microseconds us) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;
    do {
        std::this_thread::yield();
    } while(std::chrono::high_resolution_clock::now() < end);
}

int main() {
    {
        int n = 0;
        std::thread t1; // t1 is not a thread
        std::thread t2(f1, n + 1); // pass by value
        std::thread t3(f2, std::ref(n)); // pass by reference
        std::thread t4(std::move(t3)); // t4 is now running f2(). t3 is no longer a thread
        t2.join();
        t4.join();
        std::cout << "Final value of n is " << n << std::endl;
    }
    {
        std::thread threads[5];
        std::cout << "Spawning 5 threads...\n";
        for(int i = 0; i < 5; ++i) {
            threads[i] = std::thread(thread_task, i + 1);
        }
        std::cout << "Done spawning threads! Now wait for them to join\n";
        for(auto& t : threads) {
            t.join();
        }
        std::cout << "All threads joined.\n";
    }
    {
        Widget w1;
        Widget w2;
        w2 = Widget();
        Widget ws[5];
        Widget w3 = Widget();
    }
    {
        std::thread t1(foo);
        std::thread::id t1_id = t1.get_id();

        std::thread t2(foo);
        std::thread::id t2_id = t2.get_id();

        std::cout << "t1's id: " << t1_id << std::endl;
        std::cout << "t2's id: " << t2_id << std::endl;

        t1.join();
        t2.join();
    }
    {
        // joinable: 检查线程是否可以被join。检查当前的线程对象是否表示了一个活动的执行线程，
        // 由默认构造函数创建的线程是不能被join的。另外，如果某个线程已经执行完任务，
        // 但是没有被join的话，该线程依然会被认为是一个活动的执行线程，因此也是可以被join的。
        std::thread t;
        std::cout << "before starting, joinable: " << t.joinable() << std::endl;

        t = std::thread(foo);
        std::cout << "after starting, joinable: " << t.joinable() << std::endl;

        t.join();
    }
    {
        // join: Join线程，调用该函数会阻塞当前线程，直到由*this所标示的线程执行玩别join才返回
        std::cout << "starting first helper...\n";
        std::thread helper1(foo);
        std::cout << "starting second helper...\n";
        std::thread helper2(foo);
        std::cout << "waiting for helpers to finish..." << std::endl;
        helper1.join();
        helper2.join();
        std::cout << "done!" << std::endl;
    }
    {
        /*
        detach: Detach线程。将当前线程对象所代表的执行实例与该线程对象分离，使得线程的执行可以单独进行。
        一旦线程执行完毕，它所分配的资源将会被释放。
        调用detach函数之后：
        1. *this不再代表任何的线程执行实例。
        2. joinable() == false
        3. get_id() == 
        另外，如果出错或者joinable() == false, 则会抛出std::system_error。
        */
        threadCaller();
        std::this_thread::sleep_for(std::chrono::seconds(5));

    }
    {
        // swap: Swap线程，交换两个线程对象所代表的底层句柄（underlying handles）
        std::thread t1(foo);
        std::thread t2(foo);
        std::cout << "thread 1 id: " << t1.get_id() << std::endl;
        std::cout << "thread 2 id: " << t2.get_id() << std::endl;

        std::swap(t1, t2);
        
        std::cout << "after std::swap(t1, t2): " << std::endl;
        std::cout << "thread 1 id: " << t1.get_id() << std::endl;
        std::cout << "thread 2 id: " << t2.get_id() << std::endl;
        
        t1.swap(t2);

        std::cout << "after t1.swap(t2): " << std::endl;
        std::cout << "thread 1 id: " << t1.get_id() << std::endl;
        std::cout << "thread 2 id: " << t2.get_id() << std::endl;

        t1.join();
        t2.join();

    }
    {
        // native_handle: 返回native handle(由于std::thread的实现与操作系统相关，因此该
        // 函数返回与std::thread具体实现相关的线程句柄，例如在符合Posix标准的平台下
        // (如Unix/Linux)是Pthread库)
        std::thread t1(f, 1), t2(f, 2);
        sched_param sch;
        int policy;
        pthread_getschedparam(t1.native_handle(), &policy, &sch);
        sch.sched_priority = 20;
        // 要求sudo
        if(pthread_setschedparam(t1.native_handle(), SCHED_FIFO, &sch)) {
            std::cout << "Failed to setschedparam: " << std::strerror(errno) << std::endl;
        }
        t1.join();
        t2.join();
    }
    {
        // hardware_concurrency[static]: 检测硬件并发性，返回当前平台的线程实现所支持的线程并发数目，
        // 但返回值仅仅只作为系统提示
        unsigned int n = std::thread::hardware_concurrency();
        std::cout << n << " concurrent threads are supported.\n";
    }
    // std::this_thread命名空间相关辅助函数介绍
    {
        // get_id：获得线程ID
        std::thread t1(foo1);
        std::thread t2(foo1);

        std::cout << "thread t1 id: " << t1.get_id() << std::endl;
        std::cout << "thread t2 id: " << t2.get_id() << std::endl;

        t1.join();
        t2.join();
    }
    {
        // yield： 当前线程放弃执行，操作系统调度另一线程继续执行
        auto start = std::chrono::high_resolution_clock::now();

        little_sleep(std::chrono::microseconds(100));

        // std::chrono::nanoseconds 纳秒 10e-9
        // std::chrono::microseconds 微秒 10e-6
        std::chrono::nanoseconds elapsed = std::chrono::high_resolution_clock::now()  - start;
        std::cout << "waited for "
                  << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()
                  << " microseconds" << std::endl;
    }
    {
        // sleep_until: 线程休眠至某个指定的时刻(time point)，该线程才被重新唤醒
        // template<class Clock, class Duration>
        // void sleep_until(const std::chrono::time_point<Clock, Duration>& sleep_time);
    }
    {
        /*
        sleep_for: 线程休眠某个指定的时间片(time span)，该线程才被重新唤醒，不过由于线程调度等原因，
        实际休眠时间可能比sleep_duration所表示的时间片更长
        template<class Rep, class Period>
        void sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration);
        */
        std::cout << "Hello waiter" << std::endl;
        std::chrono::milliseconds dura(2000);
        std::this_thread::sleep_for(dura);
        std::cout << "Waited 2000 ms.\n";
    }
    return 0;
}