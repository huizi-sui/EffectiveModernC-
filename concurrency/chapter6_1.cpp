#include <iostream>
#include <thread>
#include <future>
#include <functional>
/*
C++11标准中与异步任务系相关的类型有下面四种
1. std::promise
2. std::packaged_task
3. std::future
4. std::shared_future
std::promise和std::packaged_task也称为异步任务的提供者Provider，
此外std::async也可以作为异步任务的提供者，不过std::async并不是类，
而是函数。
在<future>中还定义了一些辅助类，例如
std::future_error, std::future_errc, std::status, std::launch
*/
/*
异步任务提供者(Provider)介绍
1. std::promise类概述
Promise对象可以保存某一类型T的值，该值可被future对象读取(可能在另一个线程中)，
因此Promise也提供了一种线程同步的手段。
在Promise对象构造时可以和一个共享状态(通常是std::future)相关联，并可以
在相关联的共享状态(std::future)上保存一个类型为T的值。
可以通过get_future来获取与该promise对象相关联的future对象，调用该函数后，
两个对象共享相同的共享状态
1. promise对象是异步Provider，它可以在某一时刻设置共享状态的值
2. future对象可以异步返回共享状态的值，或者在必要的情况下阻塞调用者并
等待共享状态标志变为ready，然后才能获取共享状态的值
*/
void print_int(std::future<int>& fut) {
    std::cout << "start get x value\n";
    int x = fut.get(); // 获取共享状态的值
    std::cout << "value: " << x << '\n';
}
void test1() {
    std::promise<int> prom; // 生成一个std::promise<int>对象
    std::future<int> fut = prom.get_future(); // 和future相关联
    std::thread t(print_int, std::ref(fut)); // 将future交给另外一个线程t
    // 主线程睡眠2s，此时线程t将阻塞在fut.get()处，等待prom.set_value()
    std::this_thread::sleep_for(std::chrono::seconds(2));
    prom.set_value(10); // 设置共享状态的值，此处和线程t保持同步
    t.join();
}
/*
std::promise构造函数
1. promise();
默认构造函数，初始化一个空的共享状态
2.  template<class Alloc>
    promise(allocator_arg_t aa, const Alloc& alloc);
带自定义内存分配器的构造函数，与默认构造函数类似，但是使用
自定义分配器来分配共享状态
3. promise(const promise&) = delete;
拷贝构造函数，被禁用
4. promise(promise&& x) noexcept;
移动构造函数

另外, std::promise的operator=没有拷贝语义，即std::promise普通的赋值操作
被禁用，operator=只有move语义，所以std::promise对象是禁止拷贝的。
*/
std::promise<int> prom;
void print_global_promise() {
    std::future<int> fut = prom.get_future();
    int x = fut.get();
    std::cout << "value: " << x << '\n';
}
void test2() {
    std::thread th1(print_global_promise);
    prom.set_value(10);
    th1.join();
    // prom被move赋值为一个新的promise对象
    prom = std::promise<int>();
    std::thread th2(print_global_promise);
    prom.set_value(20);
    th2.join();
}
/*
std::promise::get_future介绍
该函数返回一个与promise共享状态相关联的future。
返回的future对象可以访问由promise对象设置在共享状态上的值或者某个异常对象。
只能从promise共享状态获取一个future对象。调用该函数之后，promise对象通常
会在某个时间点准备好(设置一个值或者一个异常对象)，如果不设置值或者异常，
promise对象在析构时会自动地设置一个future_error异常(broken_promise)来设置
其自身地准备状态。
*/
/*
std::promise::set_value介绍
1.  generic template:
    void set_value(const T& val);
    void set_value(T&& val);
2.  specializations
    // when T is a reference type(R&)
    void promise<R&>::set_value(R& val);
    // when T is void
    void promise<void>:: set_value(void);
设置共享状态的值，此后promise的共享状态标志变为ready
*/
/*
std::promise::set_exception介绍
为promise对象设置异常，此后promise的共享状态标志变为ready
示例如下：
线程1从终端接收一个整数，线程2将该整数打印出来，
如果线程1接收一个非整数，则为promise设置一个异常(failbit),
线程2在std::future::get时抛出该异常
*/
void get_int(std::promise<int>& prom) {
    int x;
    std::cout << "Please, enther an integer value: ";
    // 设置std::cin的异常掩码，如果发生错误，则抛出std::ios::failbit异常
    std::cin.exceptions(std::ios::failbit); // throw an failbit
    try {
        std::cin >> x;
        prom.set_value(x);
    } catch(std::exception&) {
        // std::current_exception()捕获当前的异常
        prom.set_exception(std::current_exception());
    }
}
void print_int3(std::future<int>& fut) {
    try {
        int x = fut.get();
        std::cout << "value: " << x << std::endl;
    } catch(std::exception& e) {
        std::cout << "[exception caught: " << e.what() << "]\n";
    }
}
void test3() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::thread th1(get_int, std::ref(prom));
    std::thread th2(print_int3, std::ref(fut));

    th1.join();
    th2.join();

}
/*
std::promise::set_value_at_thread_exit介绍
设置共享状态的值，但是不将共享状态的标志设置成ready，
当线程退出时该promise对象会自动设置为ready。如果某个
std::future对象与该promise对象的共享状态相关联，并且该future对象
正在调用get，则调用get的线程会被阻塞，当线程退出时，调用future::get
的线程解除阻塞，同时get返回set_value_at_thread_exit所设置的值。

该函数已经设置了promise共享状态的值，如果在线程结束之前有其他
设置或者修改共享状态的值的操作，则会抛出future_error(promise_already_satisfied)
*/
/*
std::promise::swap介绍
交换promise的共享状态
*/
int main() {
    // test1();
    // test2();
    test3();

    return 0;
}