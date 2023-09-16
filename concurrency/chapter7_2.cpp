#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
/*
std::atomic_flag过于简单，只提供了test_and_set和clear两个API，
不能满足其他需求(如store，load，exchange，compare_exchange等)
*/
/*
std::atomic基本介绍
模板类，封装一个类型为T的值
template<class T> struct atomic;
原子类型对象的主要特点是从不同线程访问不会导致数据竞争(data race)。
因此从不同线程访问某个原子对象是良心(well-defined)行为，而通常
对于非原子类型而言，并发访问某个对象(如果不做任何同步操作)会导致
未定义(undifined)行为发生。

另外，C++11标准库std::atomic提供了针对整型(interger)和指针类型的特化实现
1. 针对整型(integer)的特化，其中integer代表了如下类型
char,signed char,unsigned char,int, unsigned int, long, unsigned long,
long long, unsigned long long, char16_t,char32_t, wchar_t

template<> struct atomic<integer> {};
2. 针对指针
template<class T> struct atomic<T*> {};
*/
/*
构造函数
1. atomic() noexcept = default;
创建的对象处于未初始化状态，对处于未初始化状态的对象可以由atomic_init函数进行初始化
2. constexpr atomic(T val) noexcept;
3. atomic(const atomic&) = delete;
*/
/*
std::atomic::operator()函数
1.  T operator=(T val) noexcept;
    T operator=(T val) volatile noexcept;
2.  atomic& operator=(const atomic&) = delete;
    atomic& operator=(const atomic&) volatile = delete;
一个类型为T的变量可以赋值给相应的原子类型变量(相当于隐式转换)，该操作是原子的，
内存序(Memory Order)默认是顺序一致性(std::memory_order_seq_cst)，如果需要指定
其他的内存序，需要使用std::atomic::store()
*/
std::atomic<int> foo(0);
void set_foo(int x) {
    foo = x; // 调用 std::atomic::operator=()
}
void print_foo() {
    while(foo == 0) {
        std::this_thread::yield();
    }
    std::cout << "foo: " << foo << '\n';
}
void test1() {
    std::thread first(print_foo);
    std::thread second(set_foo, 10);
    first.join();
    second.join();
}
/*
基本std::atomic类型操作
(除了基本的外，还针对整型和指针类型做了特化，支持更多的操作)

1. is_lock_free
bool is_lock_free() const volatile noexcept;
bool is_lock_free() const noexcept;
判断是否具备lock-free特性。如果某个对象满足lock-free特性，在多个线程
访问该对象时不会导致线程阻塞(可能使用某种事务内存transactional memory
方法实现lock-free特性)

2. store
void store(T val, memory_order sync = memory_order_seq_cst) volatile noexcept;
void store(T val, memory_order sync = memory_order_seq_cst) noexcept;
修改被封装的值。sync指定内存序，可取的值：
memory_order_relaxed    Relaxed
memory_order_release    Release
memory_order_seq_cst    Sequentially consistent
3. load
T load(memory_order sync = memory_order_seq_cst) const volatile noexcept;
T load(memory_order sync = memory_order_seq_cst) const noexcept;
memory_order_relaxed    Relaxed
memory_order_consume    Comsume
memory_order_acquire    Acquire
memory_order_seq_cst    Sequentially consistent
*/
std::atomic<int> foo2(0);
void set_foo2(int x) {
    foo.store(x, std::memory_order_relaxed);
}
void print_foo2() {
    int x;
    do {
        x = foo.load(std::memory_order_relaxed);
    } while(x == 0);
    std::cout << "foo: " << x << '\n';
}
void test2() {
    std::thread first(print_foo2);
    std::thread second(set_foo2, 10);
    first.join();
    second.join();
}
/*
4. operator T
operator T() const volatile noexcept;
operator T() const noexcept;
与load功能类似，也是读取被封装的值，是类型转换操作，
默认的内存序是std::memory_order_seq_cst，如果需要指定其他的
内存序，应该使用load()函数
*/
std::atomic<int> foo3(0);
std::atomic<int> bar3(0);
void set_foo3(int x) {
    foo = x;
}
void copy_foo_to_bar() {
    // 在foo == 0时，实际上隐含了类型转换操作
    // 因此包含了operator T() const的调用
    while(foo == 0) {
        std::this_thread::yield();
    }
    // 调用operator T() const，将foo强转成int类型
    // 然后调用operator=()
    bar3 = static_cast<int>(foo);
}
void print_bar3() {
    while(bar3 == 0) {
        std::this_thread::yield();
    }
    std::cout << "bar3 = " << bar3 << '\n';
}
void test3() {
    std::thread first(print_bar3);
    std::thread second(set_foo3, 10);
    std::thread third(copy_foo_to_bar);
    first.join();
    second.join();
    third.join();
}
/*
5. exchange
T exchange(T val, memory_order sync = memory_order_seq_cst) volatile noexcept;
T exchange(T val, memory_order sync = memory_order_seq_cst) noexcept;
读取并修改被封装的值，exchange会将val指定的值替换掉之前该原子对象封装的值，并返回
之前该原子对象封装的值，整个过程是原子的。sync参数可取的内存序类型有
Relaxed、Comsume、Acquire、Release、Acquire/Release、Sequentially consistent
*/
std::atomic<bool> ready(false);
std::atomic<bool> winner(false);
void count1m(int id) {
    while(!ready) {

    }
    for(int i = 0; i < 10000000; ++i) {

    }
    if(!winner.exchange(true)) {
        std::cout << "thread #" << id << " won.\n";
    }
} 
void test4() {
    using namespace std;
    vector<thread> threads;
    for(int i = 1; i <= 10; ++i) {
        threads.emplace_back(count1m, i);
    }
    ready = true;
    for(auto& th : threads) {
        th.join();
    }
}
/*
6. compare_exchage_weak
(1).  bool compare_exchange_weak(T& excepted, T val, memory_order sync = 
                memory_order_seq_cst) volatile noexcept;
    bool compare_exchange_weak(T& excepted, T val, memory_order sync = 
                memory_order_seq_cst) noexcept;
(2).  bool compare_exchange_weak(T& excepted, T val, memory_order success,
                memory_order failure) volatile noexcept;
    bool compare_exchange_weak(T& excepted, T val, memory_order success,
                memory_order failure) noexcept;
比较并交换被封装的值(weak)与参数excepted所指定的值是否相等，如果：
1. 相等，则用val替换原子对象的旧值
2. 不相等，则用原子对象的旧值替换excepted

在(2)中，内存序的选取取决于比较操作的结果，如果比较结果为true(即原子对象的值等于expected)，
则选择参数success指定的内存序，否则选择参数failure指定的内存序

该函数直接比较原子对象封装的值与参数expected的物理内容，所以在某些情况下，对象的比较操作
在使用operator==()判断时相等，但是compare_exchange_weak判断时却可能失败，因为对象底层的
物理内容中可能存在位对齐或其他逻辑表示相同但物理表示不同的值(比如ture和2或3，它们在逻辑上
都表示“真”，但是在物理上两者的表示并不相同)

与compare_exchange_strong不同，weak版本的compare-and-exchange操作允许(虚假地)返回false
(即原子对象所封装的值与参数expected的物理内容相同，但仍返回false)，不过在某些需要循环操作的
算法下这是可以接受的，并且在一些平台下weak版本的性能更好。如果weak版本的判断确实发生了
伪失败(spurious failures)——返回false，且参数expected的值不会改变。
对于某些不需要采用循环操作的算法而言，通常采用compare_exchange_strong更好。
sync参数可取Relaxed、Comsume、Acquire、Release、Acquire/Release、Sequentially consistent
*/
struct Node {
    int value;
    Node* next;
};
std::atomic<Node*> list_head(nullptr);
void append(int val) {
    Node* newNode = new Node{val, list_head};

    // next is the same as: list_head = newNode, but in a thread-safe way:
    while(!list_head.compare_exchange_weak(newNode->next, newNode)) {

    }
    
}
void test5() {
    using namespace std;
    vector<thread> threads;
    for(int i = 0; i < 10; ++i) {
        threads.emplace_back(append, i);
    }
    for(auto& th : threads) {
        th.join();
    }

    for(Node* it = list_head; it != nullptr; it = it->next) {
        std::cout << ' ' << it->value;
    }
    std::cout << std::endl;

    Node* it;
    while(it = list_head) {
        list_head = it->next;
        delete it;
    }
} 
/*
7. compare_exchage_strong
(1).  bool compare_exchange_strong(T& excepted, T val, memory_order sync = 
                memory_order_seq_cst) volatile noexcept;
    bool compare_exchange_strong(T& excepted, T val, memory_order sync = 
                memory_order_seq_cst) noexcept;
(2).  bool compare_exchange_strong(T& excepted, T val, memory_order success,
                memory_order failure) volatile noexcept;
    bool compare_exchange_strong(T& excepted, T val, memory_order success,
                memory_order failure) noexcept;
比较并交换被封装的值(strong)与参数excepted所指定的值是否相等，如果：
1. 相等，则用val替换原子对象的旧值
2. 不相等，则用原子对象的旧值替换excepted

在(2)中，内存序的选取取决于比较操作的结果，如果比较结果为true(即原子对象的值等于expected)，
则选择参数success指定的内存序，否则选择参数failure指定的内存序

该函数直接比较原子对象封装的值与参数expected的物理内容，所以在某些情况下，对象的比较操作
在使用operator==()判断时相等，但是compare_exchange_strong判断时却可能失败，因为对象底层的
物理内容中可能存在位对齐或其他逻辑表示相同但物理表示不同的值(比如ture和2或3，它们在逻辑上
都表示“真”，但是在物理上两者的表示并不相同)

与compare_exchange_weak不同，strong版本的compare-and-exchange操作不允许(虚假地)返回false，
即原子对象所封装的值与参数expected的物理内容相同，比较操作一定返回true。
对于某些不需要采用循环操作的算法而言，通常采用compare_exchange_strong更好。
不过在某些平台下，如果算法本身需要循环操作来做检查， compare_exchange_weak 的性能会更好。
对于某些不需要采用循环操作的算法而言，通常采用compare_exchange_strong更好。
sync参数可取Relaxed、Comsume、Acquire、Release、Acquire/Release、Sequentially consistent
*/
std::atomic<Node*> list_head_strong(nullptr);
void append_strong(int val) {
    Node* newNode = new Node{val, list_head};
    while(!list_head.compare_exchange_strong(newNode->next, newNode)) {

    }
}
void test6() {
    using namespace std;
    vector<thread> threads;
    for(int i = 0; i < 10; ++i) {
        threads.emplace_back(append_strong, i);
    }
    for(auto& th : threads) {
        th.join();
    }

    for(Node* it = list_head; it != nullptr; it = it->next) {
        std::cout << ' ' << it->value;
    }
    std::cout << std::endl;
    Node* it;
    while(it = list_head) {
        list_head = it->next;
        delete it;
    }
}
int main() {
    // test1();
    // test2();
    // test3();
    // test4();
    // test5();
    test6();
}