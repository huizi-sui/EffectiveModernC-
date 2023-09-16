#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <future>
/*
std::packaged_task包装一个可调用对象，并且允许异步获取该可调用对象产生
的结果，从包装可调用对象意义上来讲，std::packaged_task与std::function类似，
只不过std::packaged_task将其包装的可调用对象的执行结果传递给一个std::future对象
(该对象通常在另外一个线程中获取std::packaged_task任务的执行结果)

std::packaged_task对象内部包含两个基本元素：
1. 被包装的任务(stored task)，任务(task)是一个可调用对象，
如函数指针、成员函数指针或者函数对象
2. 共享状态(shared status)，用于保存任务的返回值，可以通过std::future对象来达到
异步访问共享状态的效果。

可以通过std::packaged_task::get_future来获取与共享状态相关联的std::future对象。
在调用该函数之后，两个对象共享相同的共享状态，具体解释如下：
1. std::packaged_task对象是异步Provider，它在某一时刻通过调用被包装的任务来设置
共享状态的值。
2. std::future对象是一个异步返回对象，通过它可以获得共享状态的值，当然在必要的时候
需要等待共享状态标志变为ready。

std::packaged_task的共享状态的生命周期一直持续到最后一个与之相关联的对象被释放或者
销毁为止。
示例
*/
// count down taking a second for each value:
int countdown(int from, int to) {
    for(int i = from; i != to; --i) {
        std::cout << i << '\n';
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Finished!\n";
    return from - to;
}
void test1() {
    // 设置 packaged_task
    std::packaged_task<int(int,int)> task(countdown);
    // 获得与packaged_task共享状态相关联的future对象
    std::future<int> ret = task.get_future();
    // 创建一个新线程完成计数任务
    std::thread th(std::move(task), 10, 0);
    // 等待任务完成并取得结果
    int value = ret.get();
    std::cout << "The countdown lasted for " << value << " seconds.\n";
    th.join();
}
/*
std::packaged_task构造函数
1. packaged_task() noexcept;
默认构造函数，初始化一个空的共享状态，并且该packaged_task对象无包装任务
2.  template<class Fn>
    explicit packaged_task(Fn&& fn);
初始化一个共享状态，并且被包装任务由参数fn指定
3.  template<class Fn, class Alloc>
    explicit packaged_task(allocator_arg_t aa, const Alloc& alloc, Fn&& fn);
带自定义内存分配器的构造函数，与2类似，但是使用自定义分配来分配共享状态
4. packaged_task(const packaged_task&) = delete;
拷贝构造函数，被禁止
5. packaged_task(packaged_task&& x) noexcept;
移动构造函数
*/ 
void test2() {
    // 默认构造函数
    std::packaged_task<int(int)> foo;
    // 使用lambda表达式初始化一个packaged_task对象
    std::packaged_task<int(int)> bar([](int x) {
        return x * 2;
    });
    // 移动赋值
    foo = std::move(bar);
    // 获取与packaged_task共享状态相关联的future对象
    std::future<int> ret = foo.get_future();

    // 产生线程，调用被包装的任务
    std::thread(std::move(foo), 10).detach();

    // 等待任务完成并获取结果
    int value = ret.get();
    std::cout << "The double of 10 is " << value << ".\n";
}
/*
std::packaged_task::valid介绍
检查当前packaged_task是否有一个有效的共享状态相关联，
对于默认构造函数产生的packaged_task对象，该函数返回false，
除非中间进行了move赋值操作或者swap操作
*/
std::future<int> launcher(std::packaged_task<int(int)>& tsk, int arg) {
    if(tsk.valid()) {
        std::future<int> ret = tsk.get_future();
        std::thread(std::move(tsk), arg).detach();
        return ret;
    }
    return std::future<int>();
}
void test3() {
    std::packaged_task<int(int)> tsk([](int x) {
        return x * 2;
    });

    std::future<int> fut = launcher(tsk, 25);
    std::cout << "The double of 25 is " << fut.get() << ".\n";

    std::packaged_task<int(int)> tsk1;
    std::future<int> fut1 = launcher(tsk1, 20);
    std::cout << "the packaged_task is not valid. the value is " << fut1.get() << std::endl;
}
/*
std::packaged_task::get_future介绍
返回一个与packaged_task对象共享状态相关的future对象。
返回的future对象可以获得由另一个线程在该packaged_task
对象的共享状态上设置的某个值或者异常
*/
/*
std::packaged_task::operator()(Args... arg)介绍
调用该packaged_task对象所包装的对象(通常为函数指针，函数对象，lambda表达式等)，
传入的参数为args。调用该函数一般发生两种情况
1. 若成功调用packaged_task所包装的对象，则返回值(如果被包装的对象有返回值的话)
被保存在packaged_task的共享状态中。
2. 若调用packaged_task所包装的对象失败，并且抛出异常，则异常也会被保存在
packaged_task的共享状态中。
以上两种情况都使共享状态标志变为ready，因此其他等待该共享状态的线程可以获取
共享状态的值或者异常并继续执行下去。
共享状态的值可以通过在future对象(由get_futrue获得)上调用get来获得。
由于被包装的任务在packaged_task构造时指定，因此调用operator()的效果由
packaged_task对象构造时所指定的可调用对象来决定：
1. 如果被包装的任务是函数指针或者函数对象，调用std::packaged_task::operator()
只是将参数传递给被包装的对象
2. 如果被包装的任务是指向类的非静态成员函数的指针，那么std::packaged_task::operator()
的第一个参数应该指定为成员函数被调用的那个对象，剩余参数作为该成员函数的参数。
3. 如果被包装的任务是指向类的静态成员函数，那么std::packaged_task::operator()
只允许单个参数(感觉不对，下面示例f2也可以成功运行)
*/
class Widget {
public:
    static int f1(int value) {
        return value * 2;
    }
    static int f2(int value1, int value2) {
        return value1 + value2;
    }
    int f3(int value) {
        return value * 3;
    }
    int f4(int value1, int value2) {
        return value1 + value2;
    }
};
void test4() {
    std::packaged_task<int(int)> tsk(Widget::f1);
    std::future<int> fut = tsk.get_future();
    // std::thread th1(std::move(tsk), 10);
    tsk(10);
    int res1 = fut.get();
    // th1.join();
    std::cout << "res1 = " << res1 << std::endl;
    std::packaged_task<int(int,int)> tsk2(Widget::f2);
    std::future<int> fut2 = tsk2.get_future();
    // std::thread th2(std::move(tsk2), 10, 20);
    tsk2(10, 20);
    int res2 = fut2.get();
    // th2.join();
    std::cout << "res2 = " << res2 << std::endl;
    Widget w;
    std::packaged_task<int(int)> tsk3(std::bind(&Widget::f3, &w, std::placeholders::_1));
    std::future<int> fut3 = tsk3.get_future();
    // std::thread th3(std::move(tsk3), 10);
    tsk3(10);
    int res3 = fut3.get();
    // th3.join();
    std::cout << "res3 = " << res3 << std::endl;
    std::packaged_task<int(int,int)> tsk4(std::bind(&Widget::f4, &w, std::placeholders::_1, std::placeholders::_2));
    std::future<int> fut4 = tsk4.get_future();
    // std::thread th4(std::move(tsk4), 10, 20);
    tsk4(10, 40);
    int res4 = fut4.get();
    // th4.join();
    std::cout << "res4 = " << res4 << std::endl;
}
/*
std::pakcaged_task::make_ready_at_thread_exit介绍
该函数会调用被包装的任务，并向任务传递参数，类似std::packaged_task的operator()成员函数。
但是，make_ready_at_thread_exit并不会立即设置共享状态的标志为ready，而是在线程退出的时候
设置共享状态的标志

如果与该 packaged_task 共享状态相关联的 future 对象在 future::get 处等待，
则当前的 future::get 调用会被阻塞，直到线程退出。
而一旦线程退出，future::get 调用继续执行，或者抛出异常。

注意，该函数已经设置了 promise 共享状态的值，如果在线程结束之前有
其他设置或者修改共享状态的值的操作，则会抛出 future_error( promise_already_satisfied )。
*/
/*
std::packaged_task::reset()介绍
重置 packaged_task 的共享状态，但是保留之前的被包装的任务。
请看例子，该例子中，packaged_task 被重用了多次：
这里也很有问题
*/
int triple(int x) {
    return x * 3;
}
void test5() {
    std::packaged_task<int(int)> tsk(triple);
    std::future<int> fut = tsk.get_future();
    std::thread(std::move(tsk), 100).detach();
    std::cout << "The triple of 100 is " << fut.get() << ".\n";

    // re-use samke task object:
    tsk.reset();
    fut = tsk.get_future();
    std::thread t(std::move(tsk), 200);
    t.join();
    std::cout << "The triple of 200 is " << fut.get() << ".\n";
}
/*
std::packaged_task::swap
交换 packaged_task 的共享状态
*/
int main() {
    // test1();
    // std::packaged_task<int(int, int)> task(countdown);
    // task(2, 0);
    // test2();
    // test3();
    // test4();
    test5();
    return 0;
}