#include <iostream>
#include <future>
#include <chrono>
#include <functional>
#include <exception>
#include <math.h>
/*
std::future详解
简单说，std::future可以用来获取异步任务的结果，因此可以把它
当成一种简单的线程间同步的手段。
std::future通常由某个Provider创建，可以把Provider想象成一个异步任务的提供者，
Provider在某个线程中设置共享状态的值，与该共享状态相关联的std::future对象调用
get(通常在另一个线程中)获取该值，如果共享状态的标志不为ready，则调用std::future::get
会阻塞当前的调用者，直到Provider设置了共享状态的值(此时共享状态的标志变为ready)，
std::future::get返回异步任务的值或者异常(如果发生了异常)

一个有效(valid)的std::future对象通常由以下3种Provider创建，并和某个共享状态相关联。
Provider可以是函数或者类，分别是
1. std::async函数
2. std::promise::get_future，成员函数
3. std::packaged_task::get_future 成员函数

一个std::future对象只有在有效的情况下才有用。由std::future默认构造函数创建的future
对象不是有效的(除非当前非有效的future对象被move赋值到另一个有效的future)
*/
bool is_prime(int x) {
    for(int i = 2; i < x; ++i) {
        if(x % i == 0) {
            return false;
        }
    }
    return true;
}
void test1() {
    std::future<bool> fut = std::async(is_prime, 4444444443);

    std::cout << "checking, please wait";
    std::chrono::milliseconds span(100);
    while(fut.wait_for(span) == std::future_status::timeout) {
        std::cout << '.';
        // std::cout << fut.valid() << std::endl;
    }
    // std::cout << "before get " << fut.valid() << std::endl;
    bool x = fut.get();
    // std::cout << "after get " << fut.valid() << std::endl;
    std::cout << "\n444444443 " << (x ? "is" : "is not") << " prime.\n";
}
/*
std::future构造函数
std::future一般由std::async, std::promise::get_future, std::packaged_task::get_future
创建，不过也提供了构造函数
1. future() noexcept;
默认构造函数
2. future(const future&) = delete;
拷贝构造函数，被禁止
3. future(future&& x) noexcept;
移动构造函数

另外，std::future的普通赋值操作也被禁止，只提供了move赋值操作。如下
std::future<int> fut; // 默认构造函数
fut = std::async(do_some_work); // 移动赋值操作
*/
/*
std::future::share()
返回一个std::shared_future对象，调用该函数后，该std::future对象本身
已经不和任何共享状态相关联，因此该std::future的状态不再是valid
*/
int do_get_value() {
    return 10;
}
void test2() {
    std::future<int> fut = std::async(do_get_value);
    std::shared_future<int> shared_fut = fut.share();

    // 共享的future对象可以被访问多次
    // 而future对象只能get一次
    std::cout << "value: " << shared_fut.get() << "\n";
    std::cout << "its double: " << shared_fut.get() * 2 << "\n";
}
/*
std::future::get
一共有3种形式
1.  T get();
2.  R& future<R&>::get(); // when T is a reference type(R&)
3.  void future<void>::get(); // when T is void
当与该std::future对象相关联的共享状态标志变为ready后，调用该函数将
返回保存在共享状态中的值，如果共享状态的标志不为ready，则调用该函数
会阻塞当前的调用者，而此后一旦共享状态的标志变为ready，get返回Provider
所设置的共享状态的值或者异常(如果抛出了异常)
*/
void get_int(std::promise<int>& prom) {
    int x;
    std::cout << "Please, enter an integer value: ";
    std::cin.exceptions(std::ios::failbit);
    try {
        std::cin >> x;
        prom.set_value(x);
    } catch(std::exception&) {
        prom.set_exception(std::current_exception());
    }
}
void print_int(std::future<int>& fut) {
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
    // thread默认通过值传递参数，意味着会复制prom和fut，而不是使用原始对象
    // std::ref是为了确保线程函数能够访问和修改传递的对象，而不是对它们进行复制
    std::thread th1(get_int, std::ref(prom));
    std::thread th2(print_int, std::ref(fut));

    th1.join();
    th2.join();
}
/*
std::future::valid()
检查当前的std::future对象是否有效，即释放与某个共享状态相关联。
一个有效的std::future对象只能通过std::async(), std::future::get_future或者
std::packaged_task::get_future来初始化。
另外由std::future默认构造函数创建的future对象是无效的，当然通过std::future的move
赋值后该future对象也可以变为valid。
*/
void test4() {
    // 默认构造，无效
    std::future<int> foo, bar;
    foo = std::async(do_get_value); // move赋值，foo变valid
    bar = std::move(foo); // move赋值，bar变为valid，而move赋值后foo变为invalid

    if(foo.valid()) {
        std::cout << "foo's value: " << foo.get() << '\n';
    } else {
        std::cout << "foo is not valid\n";
    }

    if(bar.valid()) {
        std::cout << "bar's value: " << bar.get() << '\n';
    } else {
        std::cout << "bar is not valid\n";
    }
}
/*
std::future::wait()
等待与当前std::future对象相关联的共享状态的标志变为ready
如果共享状态的标志不是ready(此时Provider没有在共享状态上设置值(或者异常))，
调用该函数会被阻塞当前线程，直到共享状态的标志变为ready

一旦共享状态的标志变为ready，wait()函数返回，当前线程被解除阻塞，但是wait()
并不读取共享状态的值或者异常。
*/
void test5() {
    std::future<bool> fut = std::async(is_prime, 194232491);

    std::cout << "Checking...\n";
    fut.wait();

    std::cout << "\n194232491 ";
    if(fut.get()) {
        std::cout << "is prime.\n";
    } else {
        std::cout << "is not prime.\n";
    }
}
/*
std::future::wait_for
与std::future::wait的功能类似，即等待与该std::future对象相关联的共享状态标志变为ready
函数原型：
template<class Rep, class Period>
future_status wait_for(const chrono::duration<Rep, Period>& rel_time) const;
wait_for()可以设置一个时间段rel_time，如果共享状态的标志在该时间段结束之前没有被
Provider设置为ready，则调用wait_for的线程被阻塞，在等待了rel_time的时间长度后
wait_for()返回，返回值如下：
1. future_status::ready: 共享状态的标志已经变为ready，即Provider在共享状态上设置了值或者异常。
2. future_status::timeout: 超时，即在规定的时间内共享状态没有变为ready
3. future_status::deferred: 共享状态包含一个deferred函数。
见test1()
*/
/*
std::future::wait_until()
与std::future::wait()类似，即等待与该std::future对象相关联的共享状态的标志变为ready。
函数原型：
template<class Rep, class Duration>
future_status wait_until(const chrono::time_point<Rep, Duration>& abs_time) const;

wait_until()可以设置一个系统绝对时间点abs_time，如果共享状态的标志在该时间点到来
之前没有被Provider设置为ready，则调用wait_until的线程被阻塞，在abs_time这一时刻
到来之后wait_until()返回，返回值如下：
1. future_status::ready: 共享状态的标志已经变为ready，即Provider在共享状态上设置了值或者异常
2. future_status::timeout: 超时，即在规定时间内共享状态的标志没有变为ready
3. future_status::deffered: 共享状态包含一个deferred函数。
*/
/*
std::shared_future介绍
std::shared_future与std::future类似，但是std::shared_future可以拷贝、
多个std::shared_future可以共享某个共享状态的最终结果(即共享状态的某个值或者异常)。
shared_future可以通过某个std::future对象隐式转换，或者通过std::future::shared()
显式转换，无论哪种转换，被转换的那个std::future对象都会变为not-valid。

构造函数
1. 默认构造函数 shared_future() noexcept;
2. 拷贝构造函数 shared_future(const shared_futuere& x);
3. 移动构造函数 shared_future(shared_future&& x) noexcept;
4. move from future shared_future(future<T>&& x) noexcept;
即从一个有效的std::future对象构造一个std::shared_future，构造之后std::future对象x变为无效。
*/
/*
std::shared_future其他成员函数：
1. operator=(): 赋值操作符，支持拷贝赋值操作和移动赋值操作
2. get(): 获取与该std::shared_future对象相关联的共享状态的值或者异常
3. valid(): 有效性检查
4. wait(): 等待与该std::shared_future对象相关联的共享状态的标志变为ready
5. wait_for(): 等待与该std::shared_future对象相关联的共享状态的标志变为ready。
(等待一段时间，超过该时间段wait_for返回)
6. wait_until(): 等待与该std::shared_future对象相关联的共享状态的标志变为ready。
(在某一时刻前等待，超过该时刻wait_until返回)
*/
/*
与异步任务相关的类型介绍
std::future_error介绍
class future_error : public logic_error;

enum class future_errc;
enum class future_status;
enum class launch;
1. std::future_errc
类型    取值    描述
1. broken_promise   0   与该std::future共享状态相关联的std::promise对象在设置值或者异常之前被销毁
2. future_already_retrieved     1   与该std::future对象相关联的共享状态的值已经被
当前Provider获取了，即调用了std::future::get函数
3. promise_already_statisfied   2   std::promise对象已经对共享状态的值设置了某一值或者异常
4. no_status    3   无共享状态
2. std::future_status
该类型主要用在std::future(或std::shared_future)中的wait_for和wait_until两个函数中
1. future_status::ready 0   共享状态的标志已经变为ready，即Provider在共享状态上设置了值或者异常
2. future_status::timeout   1   超时，即在规定时间内共享状态的标志没有变为ready
3. future_status::deffered  2   共享状态包含一个deferred函数。
3. std::launch
该枚举类型主要是在调用std::async设置异步任务的启动策略的
1. launch::async    Asynchronous:异步任务会在另外一个线程中调用，并通过共享状态返回异步
任务的结果(一般是调用std::future::get()获取异步任务的结果)
2. launch::deferred:    Deferred:异步任务将会在共享状态被访问时调用，相当于按需调用(即延迟(deferred)调用)
*/
void do_print_ten(char c, int ms) {
    for(int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        std::cout << c;
    }
}
void test6() {
    std::cout << "with launch::async:\n";
    std::future<void> foo = std::async(std::launch::async, do_print_ten, '*', 100);
    std::future<void> bar = std::async(std::launch::async, do_print_ten, '@', 200);
    // async "get"(wait for foo and bar to be ready):
    foo.get();
    bar.get();
    std::cout << "\n\n";

    std::cout << "with launch::deferred:\n";
    foo = std::async(std::launch::deferred, do_print_ten, '*', 100);
    bar = std::async(std::launch::deferred, do_print_ten, '@', 200);
    // deferred "get"(perform the actual calls): 
    foo.get();
    bar.get();
    std::cout << '\n';
}
/*
异步任务辅助函数介绍
std::async()函数介绍
与std::future相关的函数主要是std::async(),原型如下：
1. unspecified policy
template<class Fn, class... Args>
future<typename result_of<Fn(Args...)>::type>
async(Fn&& fn, Args&&... args);
2. specific policy
template<class Fn, class... Args>
future<typename result_of<Fn(Args...)>::type>
async(launch policy, Fn&& fn, Args&&... args);
第一类没有指定异步任务(即执行某一函数)的启动策略(launch policy)
指定启动策略的函数的policy参数可以是launch::async, launch::deferred,
以及两者的按位或(|)。
launch::deferred策略不会创建新线程来执行异步任务，一般是在当前线程中执行。
采用按位或，由系统根据实际情况来做出最佳决策，提高程序性能。

std::async的fn和args参数用来指定异步任务以及参数。另外std::async返回一个
std::future对象，通过该对象可以获取异步任务的值或者异常。
介绍std::async的用法
*/
double ThreadTask(int n) {
    std::cout << std::this_thread::get_id() << " start computing..." << std::endl;
    double ret = 0;
    for(int i = 0; i < n; ++i) {
        ret += std::sin(i);
    }
    std::cout << std::this_thread::get_id() << " finished computing..." << std::endl;
    return ret;
}
void test7() {
    std::future<double> f(std::async(std::launch::async, ThreadTask, 100000000));
    # if 0 
        while(f.wait_until(std::chrono::system_clock::now() + std::chrono::second(1))
                    != std::future_status::ready) {
            std::cout << "task is running...\n";
        }
    # else
        while(f.wait_for(std::chrono::seconds(1)) != std::future_status::ready) {
            std::cout << "task is running...\n";
        }
    # endif
    std::cout << f.get() << std::endl;
}
int main() { 
    // test1();
    // test2();
    // test3();
    // test4();
    // test5();
    // test6();
    test7();
}