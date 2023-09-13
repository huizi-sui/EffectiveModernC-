#include <iostream>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <chrono>
#include <vector>
/*
基于C++11的两种基本锁类型
1. std::lock_guard， 与Mutex RAII相关，方便线程对互斥量上锁
2. std::unique_lock，与Mutex RAII相关，方便线程对互斥量上锁，
但提供了更好的上锁和解锁控制。

还提供了几个与锁类型相关的Tag类。
1. std::adopt_lock_t, 一个空的标记类
struct adopt_lock_t {};
该类型的常量对象adopt_lock
constexpr adopt_lock_t adopt_lock {};
通常作为参数传入unique_lock或lock_guard的构造函数
2. std::defer_lock_t，一个空的标记类
struct defer_lock_t {};
该类型的常量对象defer_lock
constexpr defer_lock_t defer_lock {};
通常作为参数传入给uniqe_lock或lock_guard的构造函数
3. std::try_to_lock_t， 一个空的标记类
struct try_to_lock_t {};
该类型的常量对象try_to_lock
constexpr try_to_lock_t try_to_lock {};
通常作为参数传入给unique_lock或lock_guard的构造函数
*/
/*
std::lock_guard介绍
它是一个C++11中定义的模板类
template<class Mutex>
class lock_guard;
lock_guard对象通常用于管理某个锁对象，在某个lock_guard对象的声明周期内，
它所管理的锁对象会一直保持上锁状态；而lock_guard的生命周期结束之后，
它所管理的锁对象会被解锁。
在lock_guard对象构造时，传入的Mutex对象会被当前线程锁住。在lock_guard
对象被析构时，它所管理的Mutex对象会自动解锁。
lock_guard对象并不负责管理Mutex对象的生命周期，只是简化了Mutex对象的上锁和解锁操作。

lock_guard构造函数
1. explicit lock_guard(mutex_type& m);
locking初始化，lock_guard对象管理Mutex对象m，并在构造时对m进行上锁(调用m.lock())
2. lock_guard(mutex_type& m, adopt_lock_t tag);
adopting初始化，lock_guard对象管理Mutex对象m，与1不同的是，Mutex对象m已被当前线程锁住
3. lock_guard(const lock_guard&) = delete;
拷贝构造，lock_guard对象的拷贝构造和移动构造均被禁用
*/
std::mutex mtx;
void print_thread_id(int id) {
    mtx.lock();
    std::lock_guard<std::mutex> lcx(mtx, std::adopt_lock);
    std::cout << "thread #" << id << "\n";
}
void test1() {
    std::thread threads[5];
    for(int i = 0; i < 5; ++i) {
        threads[i] = std::thread(print_thread_id, i + 1);
    }
    for(auto& th : threads) {
        th.join();
    }
}
/*
lock_guard最大的特定就是安全易于使用，在异常抛出的时候通过lock_guard
对象管理的Mutex可以得到正确的解锁
*/
void print_even(int x) {
    if(x % 2 == 0) {
        std::cout << x << " is even\n";
    } else {
        throw std::logic_error("not  even");
    }
}
void print_thread_id2(int id) {
    try {
        std::lock_guard<std::mutex> lck(mtx);
        print_even(id);
    } catch(std::logic_error&) {
        std::cout << "[exception caught]\n";
    }
}
void test2() {
    std::thread threads[5];
    for(int i = 0; i < 5; ++i) {
        threads[i] = std::thread(print_thread_id2, i + 1);
    }
    for(auto& th : threads) {
        th.join();
    }
}
/*
std::unique_lock介绍
lock_guard最大的特点就是安全简单，最大的缺点也是简单，没有足够的灵活性。
unique_lock在lock_guard的基础上提供了更好的上锁和解锁控制。

unique_lock对象以独占所有权的方式管理mutex对象的上锁和解锁操作，即没有其他
的unique_lock对象同时拥有某个mutex的所有权。

构造函数
1. unique_lock() noexcept;
默认构造函数，创建的对象不管理任何Mutex对象
2. explicit unique_lock(mutex_type& m);
locking初始化，创建的对象管理Mutex对象m，并尝试调用m.lock()对Mutex对象上锁，
如果此时另外某个unique_lock对象已经管理了该Mutex对象m，则当前线程将会被阻塞。
3. unique_lock(mutex_type& m, try_to_lock_t tag);
try-locking初始化，尝试调用m.try_lock()对Mutex对象上锁，但如果上锁不成功，
并不会阻塞当前线程
4. unique_lock(mutex_type& m, defer_lock_t tag) noexcept;
deferred初始化，在初始化的时候不会锁住Mutex对象。m应该是一个没有当前线程锁住的Mutex对象。
5. unique_lock(mutex_type& m, adopt_lock_t tag);
m应该是一个已经被当前线程锁住的Mutex对象
6.  template<class Rep, class Period>
    unique_lock(mutex_type& m, const chrono::duration<Rep, Period>& rel_time);
locking一段时间。试图通过调用m.try_lock_for(rel_time)来锁住Mutex对象一段时间
7.  template<class Clock, class Duration>
    unique_lock(mutex_type& m, const chrono::time_point<Clock,Duration>& abs_time);
locking直到某个时间点。试图通过m.try_lock_until(abs_time)来在某个时间点之前锁住对象
8. unique_lock(const unique_lock&) = delete;
拷贝构造被禁止
9. unique_lock(unique_lock&& x);
移动构造，新创建的对象获得了由x所管理的Mutex对象的所有权，包括当前Mutex的状态，调用移动构造后，
x对象不再管理任何Mutex对象了。
*/
std::mutex foo, bar;
void task_a() {
    // lock
    std::lock(foo, bar);
    // 独占锁，且已经上锁
    std::unique_lock<std::mutex> lck1(foo, std::adopt_lock);
    std::unique_lock<std::mutex> lck2(bar, std::adopt_lock);
    std::cout << "task a\n";
}
void task_b() {
    std::unique_lock<std::mutex> lck1, lck2;
    // 独占锁，还没有上锁
    lck1 = std::unique_lock<std::mutex>(bar, std::defer_lock);
    lck2 = std::unique_lock<std::mutex>(foo, std::defer_lock);
    std::lock(foo, bar);
    std::cout << "task b\n";
}
void test3() {
    std::thread th1(task_a);
    std::thread th2(task_b);
    th1.join();
    th2.join();
}
/*
移动赋值操作
1. unique_lock& operator=(unique_lock&& x) noexcept;
2. unique_lock& operator=(const unique_lock&) = delete;
移动赋值后，由x管理的Mutex对象及其状态会被新的std::unqie_lock对象取代
如果被赋值的对象之前已经获得了它锁管理的Mutex对象的锁，则在移动赋值之前
会调用unlock函数释放它锁占有的锁
*/
void print_fifty(char c) {
    std::unique_lock<std::mutex> lck;
    // move-assigned
    lck = std::unique_lock<std::mutex>(mtx);
    for(int i = 0; i < 50; ++i) {
        std::cout << c;
    }
    std::cout << "\n";
}
void test4() {
    std::thread t1(print_fifty, '*');
    std::thread t2(print_fifty, '$');
    t1.join();
    t2.join();
}
/*
主要成员函数
1. 上锁/解锁操作：lock, try_lock, try_lock_for, try_lock_until, unlock
2. 修改操作：移动赋值， 交换swap(与另一个std::unique_lock对象交换它们所管理的Muetx对象的所有权)，
释放(release)(返回指向它所管理的Mutex对象的指针，并释放所有权)
3. 获取属性操作：owns_lock(返回当前对象是否获得锁)、 operator_bool()(与owns_lock功能相同，返回
当前std::unique_lock对象是否获得了锁)、mutex(返回当前std::unque_lock对象所管理的Mutex对象指针)

std::unique_lock::lock
调用它所管理的Mutex对象的lock函数。如果在调用Mutex对象的lock函数时该Mutex对象已经被另一个线程锁住，
则该线程会被阻塞，直到它获得了锁
该函数返回时，当前的unique_lock对象便拥有了它所管理的Mutex对象的锁。如果上锁操作失败，则抛出system_error异常
*/
void print_thread_id5(int id) {
    std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
    lck.lock();
    std::cout << "thread #" << id << std:: endl;
    lck.unlock();
}
void test5() {
    std::thread threads[5];
    for(int i = 0; i < 5; ++i) {
        threads[i] = std::thread(print_thread_id5, i + 1);
    }
    for(auto& th : threads) {
        th.join();
    }
}
/*
st::unqie_lock::try_lock
调用锁管理的Mutex对象的try_lock函数，上锁成功则返回true，否则返回false
*/
void print_star() {
    std::unique_lock<std::mutex> lck(mtx, std::defer_lock);
    if(lck.try_lock()) {
        std::cout << '*';
    } else {
        std::cout << 'x';
    }
}
void test6() {
    std::vector<std::thread> threads;
    for(int i = 0; i < 500; i++) {
        threads.emplace_back(print_star);
    }
    for(auto& x : threads) {
        x.join();
    }
}
/*
std::unique_lock::try_lock_for
上锁操作，调用它所管理的 Mutex 对象的 try_lock_for 函数，如果上锁成功，则返回 true，否则返回 false。
*/
std::timed_mutex tmtx;
void fireworks() {
  std::unique_lock<std::timed_mutex> lck(tmtx, std::defer_lock);
  // waiting to get a lock: each thread prints "-" every 200ms:
  while (!lck.try_lock_for(std::chrono::milliseconds(200))) {
    std::cout << "-";
  }
  // got a lock! - wait for 1s, then this thread prints "*"
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  std::cout << "*\n";
}
void test7() {
    std::thread threads[5];
    for(int i = 0; i < 5; ++i) {
        threads[i] = std::thread(fireworks);
    }
    for(auto& th : threads) {
        th.join();
    }
}
/*
std::unique_lock::try_lock_until
上锁操作，调用它所管理的 Mutex 对象的 try_lock_until 函数，如果上锁成功，则返回 true，否则返回 false。

std::unique_lock::unlock
解锁操作，调用它所管理的 Mutex 对象的 unlock 函数。

std::unique_lock::release
返回指向它所管理的 Mutex 对象的指针，并释放所有权。
*/
int count = 0;
void print_count_and_unlock(std::mutex* p_mtx) {
    std::cout << "count: " << count << '\n';
    p_mtx->unlock();
}
void task() {
    std::unique_lock<std::mutex> lck(mtx);
    ++count;
    print_count_and_unlock(lck.release());
}
void test8() {
    std::vector<std::thread> threads;
    for(int i = 0; i < 10; i++) {
        threads.emplace_back(task);
    }
    for(auto& x : threads) {
        x.join();
    }
}
/*
std::unique_lock::owns_lock
返回当前 std::unique_lock 对象是否获得了锁。
*/
void print_star9() {
    std::unique_lock<std::mutex> lck(mtx, std::try_to_lock);
    if(lck.owns_lock()) {
        std::cout << "*";
    } else {
        std::cout << "x";
    }
}
void test9() {
    std::vector<std::thread> threads;
    for (int i=0; i<500; ++i) {
        threads.emplace_back(print_star9);
    }
    for (auto& x: threads) {
        x.join();
    }
}
/*
std::unique_lock::operator bool()
与 owns_lock 功能相同，返回当前 std::unique_lock 对象是否获得了锁。
*/
void print_star10() {
    std::unique_lock<std::mutex> lck(mtx, std::try_to_lock);
    if(lck) {
        std::cout << "*";
    } else {
        std::cout << "x";
    }
}
void test10() {
    std::vector<std::thread> threads;
    for (int i=0; i<500; ++i) {
        threads.emplace_back(print_star10);
    }
    for (auto& x: threads) {
        x.join();
    }
}

/*
std::unqie_lcok::mutex
返回当前 std::unique_lock 对象所管理的 Mutex 对象的指针。
*/
class MyMutex : public std::mutex {
private:
    int _id;
public:
    MyMutex(int id) : _id(id) {}
    int id() {
        return _id;
    } 
};
MyMutex mymtx(101);
void print_ids(int id) {
    std::unique_lock<MyMutex> lck(mymtx);
    std::cout << "thread #" << id << " locked mutex " << lck.mutex()->id() << std::endl;
}
void test11() {
    std::thread threads[10];
    for(int i = 0; i < 10; i++) {
        threads[i] = std::thread(print_ids, i + 1);
    }
    for(auto& x : threads) {
        x.join();
    }
}

int main() {
    // test1();
    // test2();
    // test3();
    // test4();
    // test5();
    // test6();
    // test7();
    // test8();
    // test9();
    // test10();
    test11();
}
