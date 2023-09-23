#include <iostream>
#include <new>
#include <ostream>
/*
item49: 了解new-handler的行为
当operator new无法满足某一内存分配需求时，会不断调用一个客户指定的错误处理函数，
即所谓的new-handler，直到找到足够内存为止，调用声明于<new>中的set_new_handler可以
指定这个函数。new_handler和set_new_handler的定义如下：
namespace std {
    using new_handler = void(*)();
    // 返回值为原来持有的new-handler
    new_handler set_new_handler(new_handler) noexcept;
}
一个良好的new-handler函数必须做以下事情之一：
1. 让更多的内存可被使用
可以让程序一开始执行就分配一大块内存，而后当new-handler第一次被调用，
将它们释还给程序使用，造成operator new的下一次内存分配动作可能成功。
2. 安装另一个new-handler
如果目前这个new-handler无法取得更多内存，可以调换为另一个可以完成目标的
new-handler(令new-handler修改“会影响new-handler行为”的静态或全局数据)
3. 卸除new-handler
将nullptr传给set_new_handler，这样会使operator new在内存分配不成功时抛出异常
4. 抛出bad_alloc(或派生自bad_alloc)的异常
这样的异常不会被operator new捕获，因此会被传播到内存分配处
5. 不返回
通过调用std::abort或std::exit

有时我们或许会希望在为不同的类分配对象时，使用不同的方式处理内存分配失败情况。
这时候使用静态成员是不错的选择
*/
class Widget {
public:
    static std::new_handler set_new_handler(std::new_handler p) noexcept;
    static void* operator new(std::size_t size);
private:
    static std::new_handler currentHandler;
};
// 做和std::set_new_handler相同的事情
std::new_handler Widget::set_new_handler(std::new_handler p) noexcept {
    std::new_handler oldHandler = currentHandler;
    currentHandler = p;
    return oldHandler;
}
void* Widget::operator new(std::size_t size) {
    // 切换至Widget的专属new-handler
    auto globalHandler = std::set_new_handler(currentHandler);
    // 分配内存或抛出异常
    void* ptr = ::operator new(size);
    // 切换回全局的new-handler
    std::set_new_handler(globalHandler);
    return ptr;
}

std::new_handler Widget::currentHandler = nullptr;

void OutOfMem() {

}
void test() {
    Widget::set_new_handler(OutOfMem);
    auto pw1 = new Widget; // 若分配失败，调用OutOfMem函数
    Widget::set_new_handler(nullptr);
    auto pw2 = new Widget; // 若分配失败，则抛出异常
}
/*
实现这一方案的代码并不因类的不同而不同，因此对这些代码加以复用是合理的构想。
一个简单的做法是建立起一个mixin风格的基类，让其派生类继承它们所需要的
set_new_handler和operator new，并且使用模板确保每一个派生类获得
一个实体互异的currentHandler成员变量。
*/
template<typename T>
class NewHandlerSupport { // minin 风格的基类
public:
    static std::new_handler set_new_handler(std::new_handler p) noexcept;
    static void* operator new(std::size_t size);
private:
    static std::new_handler currentHandler;
};

template<typename T>
std::new_handler NewHandlerSupport<T>::set_new_handler(std::new_handler p) noexcept {
    std::new_handler oldHandler = currentHandler;
    currentHandler = p;
    return oldHandler;
}
template<typename T>
void* NewHandlerSupport<T>::operator new(std::size_t size) {
    auto globalHandler = std::set_new_handler(currentHandler);
    void* ptr = ::operator new(size);
    std::set_new_handler(globalHandler);
    return globalHandler;
}

template<typename T>
std::new_handler NewHandlerSupport<T>::currentHandler = nullptr;

class Widget1 : public NewHandlerSupport<Widget1> {
public:
    // ...

};
/*
此处的模板参数T并没有真正被当成类型使用，而仅仅是用来区分不同的派生类，使得模板
机制为每个派生类具现化一份对应的currentHandler。

这个做法用到了所谓的CRTR(curious recurring template pattern，奇异递归模板模式)，
除了在上述设计模式中用到之外，它也被用于实现静态多态
*/
template<class Derived>
struct Base {
    // 在基类中暴露接口
    void Interface() {
        static_cast<Derived*>(this)->Implementation();
    }
};
struct Derived : Base<Derived> {
    // 在派生类中提供实现
    void Implementation();
};
/*
除了会调用new-handler的operator new以外，C++还保留了
传统的“分配失败便返回空指针”的operator new，称为nothrow new，
通过std::nothrow对象来使用它
*/
class Widget2 {

};
void test1() {
    // 如分配失败，抛出bad_alloc
    Widget2* pw1 = new Widget2;
    if(pw1 =nullptr) {
        // 肯定不会走这个分支
    }
    // 如果分配失败，则返回空指针
    Widget2* pw2 = new (std::nothrow) Widget2;
    if(pw2 == nullptr) {
        // 可能走这个分支
    }
    /*
    nothrow new对异常的强制保证性并不高，使用它只能保证operator new不抛出异常，
    而无法保证像new (std::nothrow) Widget这样的表达式不会导致异常，因此实际上
    并没有使用nothrow new的必要
    */
}

/*
item50：了解new和delete的合理替换时机
以下是常见的替换默认operator new和operator delete的理由

1. 用来检测运用上的错误
如果将new所得内存delete掉却不幸失败，会导致内存泄露；如果在new所得内存上
多次delete则会导致未定义行为。
如果令operator new持有一串动态分配所得地址，而operator delete将地址从中移除，
就很容易检测处出上述错误用法。
此外各式各样的编程错误可能导致overruns(写入点在分配区块尾端之后)和underruns(写入点在
分配区块起点之前)，以额外空间放置特定的byte pattern签名，检查签名是否原封不动可以检测此类错误。
*/
static const int signature = 0xDEADBEEF; // 调试魔数
using Byte = unsigned char;

void* operator new(std::size_t size) {
    using namespace std;
    // 分配额外空间以塞入两个签名
    size_t realSize = size + 2 * sizeof(int);
    void* pMem = malloc(realSize);
    if(!pMem) {
        throw bad_alloc();
    }

    // 将签名写入内存起点和尾端
    *(static_cast<int*>(pMem)) = signature;
    *(reinterpret_cast<int*>(static_cast<Byte*>(pMem) + realSize - sizeof(int))) = signature;

    // 返回指针指向第一个签名后的内存位置
    return static_cast<Byte*>(pMem) + sizeof(int);
}
// 实际上这段代码不能保证内存对齐，且有许多地方不遵守C++规范，将在item51进行详细讨论

/*
2. 为了收集使用上的统计数据：
定制 new 和 delete 动态内存的相关信息：分配区块的大小分布，寿命分布，
FIFO（先进先出）、LIFO（后进先出）或随机次序的倾向性，
不同的分配/归还形态，使用的最大动态分配量等等。

3. 为了增加分配和归还的速度：
泛用型分配器往往（虽然并非总是）比定制型分配器慢，特别是当定制型分配器专门
针对某特定类型之对象设计时。类专属的分配器可以做到“区块尺寸固定”，
例如 Boost 提供的 Pool 程序库。又例如，编译器所带的内存管理器是线程安全的，
但如果你的程序是单线程的，你也可以考虑写一个不线程安全的分配器来提高速度。
当然，这需要你对程序进行分析，并确认程序瓶颈的确发生在那些内存函数身上。

4. 为了降低缺省内存管理器带来的空间额外开销：
泛用型分配器往往（虽然并非总是）还比定制型分配器使用更多内存，
那是因为它们常常在每一个分配区块身上招引某些额外开销。
针对小型对象而开发的分配器（例如 Boost 的 Pool 程序库）本质上消除了这样的额外开销。

5. 为了弥补缺省分配器中的非最佳内存对齐（suboptimal alignment）：
许多计算机体系架构要求特定的类型必须放在特定的内存地址上，如果没有奉行这个约束条件，
可能导致运行期硬件异常，或者访问速度变低。
std::max_align_t用来返回当前平台的最大默认内存对齐类型，对于malloc分配的内存，
其对齐和max_align_t类型的对齐大小应当是一致的，
但若对malloc返回的指针进行偏移，就没有办法保证内存对齐。

6. 为了将相关对象成簇集中：
如果你知道特定的某个数据结构往往被一起使用，而你又希望在处理这些数据时
将“内存页错误（page faults）”的频率降至最低，那么可以考虑为此数据结构创建一个堆，
将它们成簇集中在尽可能少的内存页上。一般可以使用 placement new 达成这个目标（见条款 52）。

7. 为了获得非传统的行为：
有时候你会希望operator new和operator delete做编译器版不会做的事情，
例如分配和归还共享内存（shared memory），而这些事情只能被 C API 完成，
则可以将 C API 封在 C++ 的外壳里，写在定制的 new 和 delete 中。
*/

/*
item51: 编写new和delete时需固守常规
我们在条款 49 中已经提到过一些operator new的规矩，
比如内存不足时必须不断调用 new-handler，如果无法供应客户申请的内存，
就抛出std::bad_alloc异常。C++ 还有一个奇怪的规定，即使客户需求为0字节，
operator new也得返回一个合法的指针，这种看似诡异的行为其实是为了简化语言其他部分。

根据这些规约，我们可以写出非成员函数版本的operator new代码

void* operator new(std::size_t size) {
    using namespace std;
    if(size == 0) {
        size = 1;
    }

    while(true) {
        if() { // 如果分配成功
            return ; // 返回指针指向分配得到的内存
        }
        // 如果分配失败，调用目前的new-handler
        auto globalHandler = get_new_handler();
        if(globalHandler) {
            (*globalHandler)();
        } else {
            throw bad_alloc();
        }
    }
}

operator new的成员函数版本一般只会分配大小刚好为类的大小的内存空间，
但是情况并不总是如此，比如假设我们没有为派生类声明其自己的operator new，
那么派生类会从基类继承operator new，这就导致派生类可以使用其基类的 new 分配方式，
但派生类和基类的大小很多时候是不同的。

处理此情况的最佳做法是将“内存申请量错误”的调用行为改为采用标准的operator new：
void* Base::operator new(std::size_t size) {
    if(size != sizeof(Base)) {
        // 转交给标准的operator new进行处理
        return ::operator new(size);
    }
}
无需处理分配大小size是否为0，因为非附属对象必须有非零大小，因此sizeof(Base) != 0

如果你打算实现operator new[]，即所谓的 array new，那么你唯一要做的一件事就是
分配一块未加工的原始内存，因为你无法对 array 之内迄今尚未存在的元素对象做任何事情，
实际上你甚至无法计算这个 array 将含有多少元素对象。

operator delete的规约更加简单，你需要记住的唯一一件事情就是 C++ 保证 “删除空指针永远安全”：
*/
void operator delete(void* rawMemory) noexcept {
    if(rawMemory == 0) {
        return;
    }
    // 归还rawMemory所指的内存
}
// operator delete的成员函数版本要多做的唯一一件事就是将大小有误的删除行为转交给标准的operator delete：
/*
void Base::operator delete(void* rawMemory, std::size_t size) noexcept {
    if (rawMemory == 0) return;
    if (size != sizeof(Base)) {
        ::operator delete(rawMemory);    // 转交给标准的 operator delete 进行处理
        return;
    }

    // 归还 rawMemory 所指的内存
}

如果即将被删除的对象派生自某个基类而后者缺少虚析构函数，
那么 C++ 传给operator delete的size大小可能不正确，
这或许是“为多态基类声明虚析构函数”的一个足够的理由，能作为对条款 7 的补充。
*/

/*
item52： 写了placement new也要些placement delete
placement new最初的含义是 接受一个指针指向对象该被构造之处 的operator new版本
在标准库中用途广泛，其中之一就使负责在vector的未使用空间上创建对象，声明如下
void* operator new(std::size_t, void* pMemory) noexcept;
讨论广义的placement new，即带有附加参数的operator new，例如
void* operator new(std::size_t, std::ostream& logStream);
auto pw = new (std::cerr) Widget;

当我们在使用 new 表达式创建对象时，共有两个函数被调用：
一个是用以分配内存的operator new，一个是对象的构造函数。
假设第一个函数调用成功，而第二个函数却抛出异常，
那么会由 C++ runtime 调用operator delete，归还已经分配好的内存。

这一切的前提是 C++ runtime 能够找到operator new对应的operator delete，
如果我们使用的是自定义的 placement new，而没有为其准备对应的 placement delete 的话，
就无法避免发生内存泄漏。因此，合格的代码应该是这样的：
*/
class Widget3 {
public:
    // placement new
    static void* operator new(std::size_t size, std::ostream& logStream) {

    }
    // delete时调用的正常operator delete
    static void operator delete(void* pMemory) {

    }
    // placement delete
    static void operator delete(void* pMemory, std::ostream& logStream) {

    }
};

/*
另一个要注意的问题是，由于成员函数的名称会掩盖其外部作用域中的相同名称（见条款 33），
所以提供 placement new 会导致无法使用正常版本的operator new.
同理，派生类中的operator new 会掩盖全局版本和继承而得的operator new版本
*/
class Base1 {
public:
	static void* operator new(std::size_t size, std::ostream& logStream) {

    }
	
};
class Derived1 : public Base1 {
public:
	static void* operator new(std::size_t size) {

    }
};

/*
为了避免名称遮掩问题，需要确保以下形式的operator new对于定制类型仍然可用，除非你的意图就是阻止客户使用它们：
void* operator(std::size_t) throw(std::bad_alloc);           // normal new
void* operator(std::size_t, void*) noexcept;                 // placement new
void* operator(std::size_t, const std::nothrow_t&) noexcept; // nothrow new

一个最简单的实现方式是，准备一个基类，内含所有正常形式的 new 和 delete：
*/

class StandardNewDeleteForms {
public:
    // normal new/delete
    static void* operator new(std::size_t size){
        return ::operator new(size);
    }
    static void operator delete(void* pMemory) noexcept {
        ::operator delete(pMemory);
    }

    // placement new/delete
    static void* operator new(std::size_t size, void* ptr) {
        return ::operator new(size, ptr);
    }
    static void operator delete(void* pMemory, void* ptr) noexcept {
        ::operator delete(pMemory, ptr);
    }

    // nothrow new/delete
    static void* operator new(std::size_t size, const std::nothrow_t& nt) {
        return ::operator new(size,nt);
    }
    static void operator delete(void* pMemory,const std::nothrow_t&) noexcept {
        ::operator delete(pMemory);
    }
};
// 凡是想以自定义形式扩充标准形式的客户，可以利用继承和using声明式（见条款 33）取得标准形式：
class Widget4 : public StandardNewDeleteForms {
public:
    using StandardNewDeleteForms::operator new;
    using StandardNewDeleteForms::operator delete;

    static void* operator new(std::size_t size, std::ostream& logStream);
    static void operator delete(void* pMemory, std::ostream& logStream) noexcept;
};

int main() {
    // auto pb = new Base1; // 无法通过编译
    auto pb = new (std::cerr) Base1;
    // auto pd = new (std::clog) Derived1; // 无法通过编译
    auto pd = new Derived1;
}