#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
/*
C++11中的与条件变量相关的类和函数。
类：std::condition_variable和std::condition_variable_any
枚举类型std::cv_status
函数 std::notify_all_at_thread_exit()
*/
/*
std::condition_variable介绍
Linux下使用Pthread库中的pthread_cond_*()函数提供与条件变量相关的功能
Windows则参考MSDN

当std::condition_variable对象的某个wait函数被调用的时候，它使用std::unique_lock
(封装std::mutex)来锁住当前线程。当前线程会一直被阻塞，直到另一个线程在相同的
std::condition_variable对象上调用了notification函数来唤醒当前线程

std::condition_variable对象通常使用std::unique_lock<std::mutex>来等待，
如果需要使用另外的lockable类型，可以使用std::condition_variable_any类。
*/
std::mutex mtx; // 全局互斥锁
std::condition_variable cv; // 全局条件变量
bool ready = false; // 全局标志位

void do_print_id(int id) {
    std::unique_lock<std::mutex> lck(mtx);
    while(!ready) {
        cv.wait(lck); // 线程阻塞
    }
    std::cout << "thread " << id << std::endl;
}
void go() {
    std::unique_lock<std::mutex> lck(mtx);
    ready = true;
    cv.notify_all(); // 唤醒所有线程
}
void test1() {
    std::thread threads[10];
    for(int i = 0; i < 10; ++i) {
        threads[i] = std::thread(do_print_id, i);
    }
    std::cout << "10 threads ready to race...\n";
    go();
    for(auto& th : threads) {
        th.join();
    }
}
/*
构造函数
1. condition_variable()
2. condition_variable(const condition_variable&) = delete;
拷贝构造函数被禁用，只提供了默认构造函数
std::condition_variable::wait()介绍
1. void wait(unique_lock<mutex>& lck);
2. template<class Predicate>
   void wait(unique_lock<mutex>& lck, Predicate pred);
当前线程调用wait()后将被阻塞(此时当前线程应该获得了锁lck)，
直到另外某个新城调用notify_*唤醒了当前线程。

在线程被阻塞时，该函数会自动调用lck.unlock()释放锁，使得其他被阻塞在锁竞争上的
线程得以继续执行。另外，一旦当前线程获得通知(notified，通常是另外某个线程调用notify_*唤醒了
当前线程)，wait()函数也是自动调用lck.lock()，使得lck的状态和wait函数被调用时相同。

在第2种情况下(即设置了Predicate)，只有当pred条件为false时调用wait()才会阻塞当前线程，并且
在收到其他线程的通知后只有当pred为true时才会被解除阻塞。因此第二种情况类似下面代码：
while(!pred()) {
    wait(lck);
}
*/
int cargo = 0;
bool shipment_available() {
    return cargo != 0;
}
// 消费者线程
void consumer(int n) {
    for(int i = 0; i < n; ++i) {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, shipment_available);
        std::cout << cargo << '\n';
        cargo = 0;
    }
}
void test2() {
    std::thread consumer_thread(consumer, 10); // 消费者线程
    // 主线程为生产者线程，生产10个物品
    for(int i = 0; i < 10; ++i) {
        while(shipment_available()) {
            // 如果有物品，暂停当前线程，将CPU让给其他线程
            std::this_thread::yield();
        }
        std::unique_lock<std::mutex> lck(mtx);
        cargo = i + 1;
        cv.notify_one();
    }
    consumer_thread.join();
}
/*
std::condition_variable::wait_for()介绍
1.  template<class Rep, class Period>
    cv_status wait_for(unique_lock<mutex>& lck, const chrono::duration<Rep, Period>& rel_time);
2.  template<ckass Rep, class Period, class Predicate>
    bool wait_for(unique_lock<mutex>& lck, const chrono::duration<Rep, Period>& rel_time, Predicate pred);
wait_for可以指定一个时间段，在当前线程收到通知或者指定的时间rel_time超时之前，该线程都会处于阻塞状态。
而一旦超时过着收到了其他线程的通知，wait_for返回。

wait_for重载版本的最后一个参数pred表示wait_for的预测条件，只有当pred条件为false时调用wait()才会阻塞当前线程，
并且在收到其他线程的通知后只有当pred为true时才会被解除阻塞，相当于如下代码：
return wait_until(lck, chrono::steady_clock::now() + rel_time, std::move(pred));
示例：主线程等待th线程输入一个值，然后将th线程从终端接收的值打印出来，在th线程接收到值之前，主线程一直等待，
每个1s超时一次，并打印一个"."
*/
int value;
void do_read_value() {
    std::cin >> value;
    cv.notify_one();
}
void test3() {
    std::cout << "Please, enter an iteger: \n";
    std::thread th(do_read_value);

    std::mutex mtx;
    std::unique_lock<std::mutex> lck(mtx);
    while(cv.wait_for(lck, std::chrono::seconds(1)) == std::cv_status::timeout) {
        std::cout << '.';
        std::cout.flush();
    }
    std::cout << "You entered: " << value << '\n';

    th.join();
}
/*
std::condition_variable::wait_until()介绍
1.  template<class Clock, class Duration>
    cv_status wait_until(unique_lock<metex>& lck, const chrono::time_point<Clock, Duration>& abs_time);
2.  template<class Clock, class Duration, class Predicate>
    bool wait_until(unique_lock<metex>& lck, const chrono::time_point<Clock, Duration>& abs_time, Predicate pred);

wait_until可以指定一个时间点，在当前线程收到通知或者指定时间点abs_time超时之前，该线程都会处于阻塞状态。
而一旦超时或者收到了其他线程的通知，wait_until返回，后续类似wait()
wait_until重载版本最后一个参数pred是wait_until的预测条件，只有pred条件为false时调用wait()才会阻塞当前线程，
且在收到其他线程通知后只有当pred为true时才会被解除阻塞，相当于如下代码：
while(!pred()) {
    if(wait_until(lck, abs_time) == cv_status::timeout) {
        return pred();
    }
}
return true;
*/
/*
std::condition_variable::notify_one()介绍
唤醒某个等待(wait)线程。如果当前没有等待线程，则该函数什么也不做。
如果同时存在多个等待线程，则唤醒某个线程是不确定的(unspecified)
*/
void consumer1() {
    std::unique_lock<std::mutex> lck(mtx);
    while(cargo == 0) {
        cv.wait(lck);
    }
    std::cout << cargo << '\n';
    cargo = 0;
}
void producer(int id) {
    std::unique_lock<std::mutex> lck(mtx);
    cargo = id;
    cv.notify_one();
}
void test4() {
    std::thread consumers[10], producers[10];
    for(int i = 0; i < 10; ++i) {
        consumers[i] = std::thread(consumer1);
        producers[i] = std::thread(producer, i + 1);
    }
    for(int i = 0; i  < 10; ++i) {
        consumers[i].join();
        producers[i].join();
    }
}
/*
std::condition_variable::notify_all()介绍
唤醒所有的等待(wait)线程。如果当前没有等待线程，则该函数什么也不做。
示例： test1()
*/

/*
std::condition_variable_any介绍
与std::condition_variable类似，只不过std::condition_variable_any的wait
函数可以接受任何lockable参数，而std::condition_variable只能接受
std::unique_lock<std::mutex>类型参数。
除此之外，和std::condition_variable几乎完全一样
*/

/*
std::cv_status枚举类型介绍
cv_status::no_timeout: wait_for或者wait_until没有超时，
即在规定的时间段内线程线程收到了通知
cv_status::timeout: wait_for或者wait_until超时
*/

/*
std::notify_all_at_thread_exit
函数原型：
void notify_all_at_thread_exit(condition_variable& cond, unique_lock<mutex> lck);
当调用该函数的线程退出时，所有在cond条件变量上等待的线程都会收到通知。
*/
void print_id5(int id) {
    std::unique_lock<std::mutex> lck(mtx);
    while(!ready) {
        cv.wait(lck);
    }
    std::cout << "thread " << id << '\n';
}
void go5() {
    std::unique_lock<std::mutex> lck(mtx);
    std::notify_all_at_thread_exit(cv, std::move(lck));
}
void test5() {
    std::thread threads[10];
    for(int i = 0; i < 10; ++i) {
        threads[i] = std::thread(print_id5, i);
    }
    std::cout << "10 threads ready to race...\n";
    std::thread(go).detach();
    for(auto& th : threads) {
        th.join();
    }
}

class Widget {
public:
    Widget() {
        std::cout << "构造函数" << std::endl;
    }
    Widget(const Widget& w) {
        std::cout << "拷贝构造" << std::endl;
    }
    Widget& operator=(const Widget& w) {
        std::cout << "拷贝赋值" << std::endl;
        return *this;
    }
    Widget& operator=(Widget&& w) {
        std::cout << "移动赋值" << std::endl;
        return *this;
    }
    Widget(Widget&& w) {
        std::cout << "移动构造" << std::endl;
    }
};

void test(Widget w) {
    std::cout << "test" << std::endl;
}
void test11() {
    Widget w;
    test(std::move(w));
    Widget w1 = Widget();
    Widget w2 = w1;
    Widget w3 = std::move(w1);
}

/*
利用条件变量进行线程同步
在多线程同步时，条件变量通常和锁一起使用。
*/
std::string data;
bool processed = false;
void worker_thread() {
    // Wait until the main thread sends data
    {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, []() {
            return ready;
        });
    }
    std::cout << "Worker thread is processing data\n";
    data += " after processing";
    // Send data back to the main thread
    {
        std::lock_guard<std::mutex> lck(mtx);
        processed = true;
        std::cout << "Worker thread signals data processing completed\n";
    }
    cv.notify_one();
}
void test6() {
    std::thread worker(worker_thread);

    data = "Example data";
    // send data to the worker thread
    {
        std::lock_guard<std::mutex> lck(mtx);
        ready = true;
        std::cout << "the main thread signals data ready for processing\n";
    }
    cv.notify_one();
    // wait for the worker
    {
        std::unique_lock<std::mutex> lck(mtx);
        cv.wait(lck, []() {
            return processed;
        });
    }
    std::cout << "Back in the main thread, data = " << data << '\n';
    worker.join();
}

int main() {
    // test1();
    // test2();
    // test3();
    // std::cout << std::numeric_limits<long long>::max() << std::endl;
    // test4();
    // test5();
    // test11();
    test6();
    return 0;
}