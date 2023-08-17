/*
item24: 区别通用引用和右值引用

请记住：
1. 如果一个函数模板形参的类型为T&&，并且T需要被推导得知，或者如果一个对象被声明为auto&&，这个形参或者对象就是一个通用引用。
2. 如果类型声明的形式不是标准的type&&，或者如果类型推导没有发生，那么type&&代表一个右值引用。
3. 通用引用，如果它被右值初始化，就会对应地成为右值引用；如果它被左值初始化，就会成为左值引用。

T&&有两种不同的意思
1. 右值引用，它们只能绑定到右值上，并且它们主要的存在原因iu是为了识别可以移动操作的对象
2. 它既可以是右值引用，也可以是左值引用。这种引用在源码里看起来像右值引用（即“T&&”），
但是它们可以表现得像是左值引用（即“T&”）。它们的二重性使它们既可以绑定到右值上（就像右值引用），
也可以绑定到左值上（就像左值引用）。
此外，它们还可以绑定到const或者non-const的对象上，也可以绑定到volatile或者non-volatile的对象上，
甚至可以绑定到既const又volatile的对象上。
它们可以绑定到几乎任何东西。这种空前灵活的引用值得拥有自己的名字。我把它叫做通用引用（universal references）

2种情况下会出现通用引用。最常见的一种是函数模板形参，第二种情况是auto声明符。
这两种情况的共同之处就是都存在类型推导。
在模板f的内部，param类型需要被推导，而在变量var2的声明中，var2的类型也需要被推导。
*/
#include <iostream>
#include <vector>

class Widget {

};
// 右值引用，没有类型推导
void f1(Widget&& param) {

}
Widget&& var1 = Widget(); // 右值引用，没有类型推导
auto&& var2 = var1; // 不是右值引用，通用引用

template<typename T>
void f2(std::vector<T>&& param) { //  右值引用，不是T&&，所以不是通用引用

}

template<typename T>
void f3(const T&& param) { // param是一个右值引用，因为const使一个引用失去成为通用引用的资格

}

template<typename T>
void f(T&& param) { // 不是右值引用，是通用引用

}

// 在一个模板内看见一个函数形参类型为T&&，它不一定是一个通用引用
// 这是由于在模板内部并不保证一定发生类型推导
// 下面是vector的简化版本
template<class T, class Allocator = std::allocator<T>>
class vector1 {
public:
    // push_back函数的形参当然有一个通用引用的正确形式，然而这里没有发生类型推导
    // 因为push_back在有一个特定的vector实例之前不可能存在。
    // 而实例化vector时的类型已经决定了push_back的声明
    void push_back(T&& x) {

    }
    // 而emplace_back确实包含类型推导
    template<class... Args>
    void emplace_back(Args&&... args) { // 通用引用

    }
};

template<>
class vector1<Widget, std::allocator<Widget>> {
public:
    void push_back(Widget&& x); // 右值引用
};

int main() {
    {
        Widget w;
        f(w); // 传递给函数模板f一个左值，param的类型将会是Widget&，即左值引用
        f(std::move(w)); // 传递给函数模板f一个右值，param的类型将会Widget&&，即右值引用
    }
    {
        std::vector<int> v;
        // f2(v); // error 不能将左值绑定到右值引用
    }
    {
        vector1<Widget> v; // 模板被实例化，等价于class vector1<Widget, std::allocator<Widget>>;
    }
}