/*
item9： 优先考虑别名声明而非typedefs

别名声明可以被模板化（别名模板alias templates），但是teypedef不能

请记住：
1. typedef不支持模板化，但是别名声明支持。
2，别名模板避免了使用“::type”后缀，而且在模板中使用typedef还需要在前面加上typename
3，C++14提供了C++11所有type traits转换的别名声明版本
*/
#include <iostream>
#include <unordered_map>
#include <memory>
#include <list>
#include <type_traits>

class Widget {
    
};

// typedef是C++98的东西
typedef std::unique_ptr<std::unordered_map<std::string, std::string>> UPtrMapSS;
// C++11版本, 别名声明
using UPtrMapSS1 = std::unique_ptr<std::unordered_map<std::string, std::string>>;

// 声明一个函数指针时，别名声明更容易理解
typedef void (*FP)(int, const std::string &);
using FP1 = void (*)(int, const std::string &); // 含义同上

// 链表, 别名模板实现
template<typename T>
using MyAllocList = std::list<T, std::allocator<T>>;

MyAllocList<Widget> lw;

// typedef需要从头实现
template<typename T>
struct MyAllocList1 {
    typedef std::list<T, std::allocator<T>> type;
};
MyAllocList1<Widget>::type lw1; // 用户代码

template<typename T>
class Widget1 {
private:
    /*
    如果想在一个模板内使用typedef声明一个链表对象，而这个对象又使用了
    模板形参，就必须在typedef前面加上typedef
    */
   // 这里MyAlloclist<T>::type使用了一个类型，这个类型依赖于模板参数T
   // 因此MyAlloclist<T>::type是一个依赖类型
   // C++规定必须在依赖类型名前加上typename
    typename MyAllocList1<T>::type list;
};

// 如果使用别名声明一个MyAllocList，就不需要使用typename
template<typename T>
class Widget2 {
private:
    MyAllocList<T> list; // 没有typename
};

/*
    当编译器处理Widget模板时遇到MyAllocList<T>，它们知道My A了咯从List<T>
    是一个类型名，因为MyAllocList是一个别名模板，它一定是一个类型名。
    因此MyAllocList<T>就是一个非依赖类型，不需要也不允许使用typename修饰符
    当编译器在Widget的模板中看到了MyAllocList1<T>::type时，它不能确定那是一个类型的名称。
    因为可能存在一个MyAllocList1的它们没见到的特化版本，那个版本的MyAllocList1<T>::type
    指代了一种不是类型的东西。

    示例：
*/
class Wine {

};

template<> // T是Wine， 特化MyAllocList1
class MyAllocList1<Wine> {
private:
    enum class WineType {
        White, Red, Rose
    };
    WineType type; // type是一个数据成员
    /*
    如果Widget使用Wine实例化，在Widget模板中的MyAllocList1<Wine>::type
    将会是一个数据成员，而不是一个类型。
    在Widget模板内，MyAllocList<T>::type是否表示一个类型取决于T是什么，
    这就是为什么编译器坚持要求你在前面加上typename
    */
};

/*
C++11在type traits（类型特性）中给了你一系列工具去实现类型转换，
如果要使用这些模板请包含头文件<type_traits>。
里面有许许多多type traits，也不全是类型转换的工具，
也包含一些可预测接口的工具。给一个你想施加转换的类型T，
结果类型就是std::transformation<T>::type，

std::remove_const<T>::type // 从const T中产出T, C++11，使用了typedef形式
std::remove_const<T> // C++14 等价形式， 使用别名声明形式
std::remove_reference<T>::type // 从T&和T&&中产出T, C++11
std::remove_reference<T> // C++14等价形式
std::add_lvalue_reference<T>::type // 从T中产出T&, C++11
std::add_lvalue_reference<T> // C++14等价形式
*/
// 别名声明很好，要在C++11中使用也很简单
template<class T>
using remove_const_t = typename std::remove_const<T>::type;

template<class T>
using remove_reference_t = typename std::remove_reference<T>::type;

template<class T>
using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;

int main() {
    const int x = 10;
}