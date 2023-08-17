/*
item11：优先考虑使用deleted函数而非使用未定义的私有声明

请记住：
1. 比起声明函数为private但不定义，使用deleted函数更好
2. 任何函数都能被删除（be deleted），包括非成员函数和模板实例（译注：实例化的函数）

代码被使用时，不想被调用某个特殊函数，通常不会声明这个函数。无声明，不函数！
但有时C++会自动声明一些函数，如果防止调用这些函数，事情就不那么简单了。
Item17详细讨论了这些函数，这里只关注拷贝构造函数和拷贝赋值函数

在C++98中，想要禁止使用的成员函数，几乎总是拷贝构造函数或者是赋值运算符，或者两者都是
在C++98中，防止调用这些函数的方法是将他们声明为私有成员函数并且不定义
例如C++标准库isostream的继承链顶部是模板类baisc_ios，所有istream和ostream类都继承此类。
拷贝istream和ostream是不合适的，C++98中是这么声明的
template<class charT, class traits = char_traits<charT>>
class basic_ios : public ios_base {
public:
    ...
private:
    basic_ios(const basic_ios&); // not define
    basic_ios& operator=(const basic_ios&); // not define
};
故意不定义它们意味着如果还是有代码用它们（如成员函数或者类的友元friend），
就会在链接时引发缺少函数定义错误

C++11中有一个中更好的方式达到相同目的，即使用delete关键字
上面代码在C++11中表达如下
template<class charT, class traits = char_traits<charT>>
class basic_ios : publuic ios_base {
public: 
    ...
    
    basic_ios(const basic_ios&) = delete;
    basic_ios& operator=(const basic_ios&) = delete;
};
delete函数不能以任何方式被调用，即使在成员函数或者友元函数中调用delete函数
也不能通过编译，这是较之C++98的改进，C++98中不正确的使用这些函数在链接时才会被诊断出来

通常delete函数被声明为public而不是private。调用成员函数时，C++会在检查delete状态前
检查它的访问行。当调用一个私有的delete函数，一些编译器只会给出该函数是private的错误。

delete函数还有一个优势： 任何函数都可以标记为deleted，而只有成员函数可以被标记为private
另外delete函数能做到，但是private成员函数作不到的是禁止一些模板的实例化

事实上C++98的最佳实践即声明函数为private但不定义是在做C++11 deleted函数要做的事情。
作为模仿者，C++98的方法不是十全十美。它不能在类外正常工作，不能总是在类中正常工作，
它的罢工可能直到链接时才会表现出来。所以请坚定不移的使用deleted函数。


*/

#include <iostream>

// 非成员函数： 判断整数是否是幸运数字
bool isLucky(int number) {
}
// 创建delete重载函数
bool isLucky(char) = delete; // 拒绝char
bool isLucky(bool) = delete; // 拒绝bool
// 将float转换为int和double， C++会更喜欢转换为double
bool isLucky(double) = delete; // 拒绝float和double

// 假设一个模板仅支持原生指针
/*
指针世界有两种特殊情况
1. void*指针，因为没有办法对它们进行解引用，或者加加减减等
2. char*指针，因为它们通常代表C风格字符串，而不是正常意义下指向单个字符的指针
这两种情况要特殊处理，假设下面函数模板应该拒绝这两个类型，也就是说processPointer不能被
void*和char*调用
*/
template<typename T>
void processPointer(T* ptr) {}

template<>
void processPointer<void>(void* ptr) = delete;
template<>
void processPointer<char>(char* ptr) = delete;
// 按常理来说const void*和const char*也应该无效
template<>
void processPointer<const void>(const void* ptr) = delete;
template<>
void processPointer<const char>(const char* ptr) = delete;
// 如果想做的更彻底一些，还需要删除const volatile void*和const volatile char*重载版本
// 还有一些其他标准字符类型的重载版本： std::wchar_t, std::wchar16_t std::char32_t

// 如果类中有一个函数模板，可能想用private（经典C++98惯例）来禁止这些函数模板实例化
// 但是不能这么做，因为不能给特化的成员模板函数指定一个不同于主函数模板的访问级别
// class Widget {
// public:
//     template<typename T>
//     void processPointer(T* ptr) {

//     }
// private:
//     template<> // error 原因是模板特例化必须位于一个命名空间作用域，而不是类作用域
//     void processPointer<void>(void*);
// };
// delete函数可以，因为它不需要一个不同的访问级别，且它们可以在类外被删除（因此位于命名空间作用域）
class Widget {
public:
    template<typename T>
    void processPointer(T* ptr) {

    }

    // template<> // error: 模板特例化必须位于同一个命名空间，而这里是类作用域
    // void processPointer<void>(void*) = delete;
};

template<>
void Widget::processPointer<void>(void*) = delete; // 还是public，且位于同一命名空间作用域

int main() {
    {
        // C++有沉重的C包袱，使得含糊的、能被视作数值的任何类型都隐式转换为int
        // 但是有一些调用可能无意义
        // if(isLucky('a')) {}
        // if(isLucky(true)) {}
        // if(isLucky(3.5)) {}
        // 如果幸运数必须是整型，就应该禁止这些调用通过编译
        // 其中一种方法就是创建delete重载函数
    }
}