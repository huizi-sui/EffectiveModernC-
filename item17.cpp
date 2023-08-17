/*
item17: 理解特殊成员函数的生成

请记住：
1. 特殊成员函数是编译器可能自动生成的函数：默认构造函数，析构函数，拷贝操作，移动操作。
2. 移动操作仅当类没有显式声明移动操作，拷贝操作，析构函数时才自动生成。
3. 拷贝构造函数仅当类没有显式声明拷贝构造函数时才自动生成，并且如果用户声明了移动操作，拷贝构造就是delete。
拷贝赋值运算符仅当类没有显式声明拷贝赋值运算符时才自动生成，并且如果用户声明了移动操作，拷贝赋值运算符就是delete。
当用户声明了析构函数，拷贝操作的自动生成已被废弃。
4. 成员函数模板不抑制特殊成员函数的生成。


在C++术语中，特殊成员函数是指C++自己生成的函数。
C++98有四个：默认构造函数，析构函数，拷贝构造函数，拷贝赋值运算符。
这些函数仅在需要的时候生成
默认构造函数仅在类完全没有构造函数的时候生成
生成的特殊成员函数是隐式public和inline的，他们是非虚的。除非函数是在派生类中的析构函数，
且继承了有虚析构函数的基类，这时，编译器为派生类生成的析构函数是虚的。

C++11特殊成员函数俱乐部迎来了两位新会员：移动构造函数和移动赋值运算符
class Widget {
public:
    Widget(Widget&& rhs); // 移动构造函数
    Widget& operator=(Widget&& rhs); // 移动赋值运算符
};
移动操作仅在需要的时候生成，如果生成了，就会对类的non-static数据成员执行逐成员的移动。
那意味着移动构造函数根据rhs参数里面对应的成员移动构造出新non-static部分，
移动赋值运算符根据参数里面对应的non-static成员移动赋值。
移动构造函数也移动构造基类部分（如果有的话），移动赋值运算符也是移动赋值基类部分。

对一个数据成员或者基类使用移动构造或者移动赋值时，没有任何保证移动一定会真的发生。
逐成员移动，实际上，更像是逐成员移动请求，因为对不可移动类型（即对移动操作没有特殊支持的类型，比如大部分C++98传统类）
使用“移动”操作实际上执行的是拷贝操作。
逐成员移动的核心时对对象使用std::move，然后函数决议时会选择执行移动还是拷贝操作。
简单记忆： 如果支持移动就会逐成员移动类成员和基类成员，不支持移动就执行拷贝操作。

两个拷贝操作是独立的：声明一个不会限制编译器生成另一个。
如果声明了一个拷贝构造函数，没有声明拷贝赋值函数，如果写的代码用到了拷贝赋值，
编译器会帮助生成拷贝赋值运算符。反过来也成立。

两个移动操作不是相互独立的。声明其中一个，编译器就不会再生成另一个。
如果在类中声明了一个移动构造函数，就表明对于移动操作应怎么实现，与编译器应生成
的默认逐成员移动有些区别。如果逐成员移动构造有些问题，那么逐成员移动赋值同样也可能有问题。
所以声明移动构造函数阻止移动赋值运算符的生成，声明移动赋值运算符同样阻止编译器生成移动构造函数。

如果一个类显式声明了拷贝操作，编译器就不会生成移动操作。
这种限制的解释是如果声明拷贝操作（构造或者赋值）就暗示着平常拷贝对象的方法（逐成员拷贝）不适用于该类，
编译器会明白如果逐成员拷贝对拷贝操作来说不合适，逐成员移动也可能对移动操作来说不合适。

这是另一个方向。声明移动操作（构造或赋值）使得编译器禁用拷贝操作。
（编译器通过给拷贝操作加上delete来保证，参见Item11。）
（译注：禁用的是自动生成的拷贝操作，对于用户声明的拷贝操作不受影响）
毕竟，如果逐成员移动对该类来说不合适，也没有理由指望逐成员拷贝操作是合适的。

Rule of Three规则：如果声明了拷贝构造函数、拷贝赋值函数、析构函数三者之一，也应该声明其余两个。
用户接管拷贝操作的需求几乎都是因为该类会做其他资源的管理，这意味着
1. 无论哪种资源管理如果在一个拷贝操作内完成，也应该在另一个拷贝操作内完成。
2. 类的析构函数也需要参与资源的管理（通常是释放）
通常要管理的资源是内存。

Rule of Three带来的后果就是只要出现用户定义的析构函数就意味着简单的逐成员拷贝操作
不适用于该类。
所以如果一个类声明了析构，拷贝操作可能不应该自动生成，因为它们做的事情可能是错误的。
在C++98提出的时候，上述推理没有得到足够的重视，所以C++98用户声明析构函数不会左右
编译器生成拷贝操作的意愿。C++11中情况仍然如此，但仅仅是因为限制拷贝操作生成的条件会破环老代码。

Rule of Three规则背后的解释依然有效，再加上对声明拷贝操作阻止移动操作隐式生成的观察，
使得C++11不会为那些有用户定义的析构函数的类生成移动操作。

所以仅当下面条件成立时才会生成移动操作（当需要时）：
1. 类中没有拷贝操作
2. 类中没有移动操作
3. 类中没有用户定义的析构

C++11抛弃了已声明了拷贝操作或析构函数的类的拷贝操作的自动生成。
这意味着如果你的某个声明了析构或者拷贝的类依赖自动生成的拷贝操作，应该考虑升级这些类，消除依赖。


总结，C++11对于特殊成员函数处理规则如下：
1. 默认构造函数：和C++98规则相同。仅当类不存在用户声明的构造函数时才自动生成。
2. 析构函数：基本上和C++98相同；稍微不同的是现在析构默认noexcept（参见Item14）。
和C++98一样，仅当基类析构为虚函数时该类析构才为虚函数。
3. 拷贝构造函数：和C++98运行时行为一样：逐成员拷贝non-static数据。
仅当类没有用户定义的拷贝构造时才生成。如果类声明了移动操作它就是delete的。
当用户声明了拷贝赋值或者析构，该函数自动生成已被废弃。
4. 拷贝赋值运算符：和C++98运行时行为一样：逐成员拷贝赋值non-static数据。
仅当类没有用户定义的拷贝赋值时才生成。如果类声明了移动操作它就是delete的。
当用户声明了拷贝构造或者析构，该函数自动生成已被废弃。
5. 移动构造函数和移动赋值运算符：都对非static数据执行逐成员移动。
仅当类没有用户定义的拷贝操作，移动操作或析构时才自动生成。
*/
#include <iostream>
#include <map>
/*
这种方法通常在多态基类中很有用，通过操作的是哪个派生类对象来定义接口。
多态基类通常有一个虚析构函数，因为如果它们非虚，一些操作（比如通过一个基类指针或者引用
对派生类对象使用delete或者typeid）会产生未定义或错误结果。
*/
class Widget {
public:
    ~Widget() {
        // 用户声明的析构
    }
    // 默认拷贝构造函数的行为还可以使用
    Widget(const Widget&) = default;
    // 默认拷贝赋值运算符的行为还可以
    Widget& operator=(const Widget&) = default;
};
/*
析构函数为虚函数的唯一方法是添加virtual关键字。
通常，默认实现是对的， = default是一个不错的方式表示默认实现。
然而用户声明析构函数会抑制编译器生成移动操作，所以
如果该类需要具有移动性，就为移动操作加上=default。
声明移动会抑制拷贝生成， 所以如果拷贝性也需要支持，再为
拷贝操作加上=default
*/
class Base {
public:
    virtual ~Base() = default;

    Base(Base&& ) = default; // 支持移动
    Base& operator=(Base&&) = default;

    Base(const Base&) = default; // 支持拷贝
    Base& operator=(const Base&) = default; 
};

class StringTable {
public:
    StringTable() {
        std::cout << "StringTable()" << std::endl;
    }
    ~StringTable() {
        std::cout << "~StringTable()" << std::endl;
    }
    StringTable(const StringTable& s) {
        values = s.values;
        std::cout << "StringTable(const StringTable&)" << std::endl;
    }
    StringTable& operator=(const StringTable& s) {
        values = s.values;
        std::cout << "StringTable& operator=(const StringTable&)" << std::endl;
        return *this;
    }
private:
    std::map<int, std::string> values;
};

class StringTable1 {
public:
    StringTable1() {
        std::cout << "StringTable1()" << std::endl;
    }
    ~StringTable1() {
        std::cout << "~StringTable1()" << std::endl;
    }
    StringTable1(const StringTable1& s) {
        values = s.values;
        std::cout << "StringTable1(const StringTable1&)" << std::endl;
    }
    StringTable1& operator=(const StringTable1& s) {
        values = s.values;
        std::cout << "StringTable1& operator=(const StringTable1&)" << std::endl;
        return *this;
    }
    StringTable1(StringTable1&& s) {
        std::cout << "StringTable1(StringTable1&&)" << std::endl;
        values = std::move(s.values);
    }
    StringTable1& operator=(StringTable1&& s) {
        std::cout << "StringTable1& operator=(StringTable1&&)" << std::endl;
        values = std::move(s.values);
        return *this;
    }
private:
    std::map<int, std::string> values;
};

// 成员函数模板不抑制特殊成员函数的生成
// 在该类中，编译器仍会生成移动和拷贝操作，即使可以模板实例化产出
// 拷贝构造和拷贝赋值运算符的函数签名（当T为Widget1时）。
class Widget1 {
public:
    template<typename T> // 从任何东西构造Widget1
    Widget1(const T& rhs) {

    }
    template<typename T> // 从任何东西赋值给Widget1
    Widget& operator=(const T& rhs) {

    }
};

int main() {
    {
        StringTable s; // 默认构造函数
        StringTable s1(s); // 拷贝构造函数
        StringTable s2{s}; // 拷贝构造函数
        StringTable s3 = s; // 拷贝构造函数
        s3 = s; // 拷贝赋值函数
        StringTable& s4 = s; // 起别名，所以不会调用任何函数

        // 因为声明了拷贝操作，抑制编译器自动生成移动操作相关函数，
        // 因此std::move函数发现该类无法进行移动，只能退化为拷贝操作
        StringTable s5 = std::move(s); // 拷贝构造函数
        StringTable s6(std::move(s)); // 拷贝构造函数
        StringTable s7{std::move(s)}; // 拷贝构造函数
        s7 = std::move(s); // 拷贝赋值函数
    }
    std::cout << "================" << std::endl;
    {
        StringTable1 s; // 默认构造函数
        StringTable1 s1(s); // 拷贝构造函数
        StringTable1 s2{s}; // 拷贝构造函数
        StringTable1 s3 = s; // 拷贝构造函数
        s3 = s; // 拷贝赋值函数
        StringTable1& s4 = s; // 起别名，所以不会调用任何函数

        StringTable1 s5 = std::move(s); // 移动构造函数
        StringTable1 s6(std::move(s)); // 移动构造函数
        StringTable1 s7{std::move(s)}; // 移动构造函数
        s7 = std::move(s); // 移动赋值函数
    }
}