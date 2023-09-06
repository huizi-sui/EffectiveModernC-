/*
item33: 对auto&&形参使用decltype以std::forward它们

泛型lambda是C++14中最值得期待的特性之一，因为在lambda的形参中可以使用auto关键字，
这个特性的实现是非常直接的：在闭包类的operator()函数是一个函数模板

请记住：
1. 对auto&&形参使用delctype以std::forward它们。
*/
#include <iostream>
#include <type_traits>

class Widget {

};

template<typename T>
T normalize(T t) {
    return t;
}

template<typename T>
int func(T t) {
    int x = 10;
    return x;
}

auto f = [](auto x) {
    //...
    // 这里lambda对x做的唯一一件事就是把它转发给函数normalize。
    // 如果函数normalize对待左值和右值的方式不一样，这里的实现就不太合适
    // 因为即使传递到lambda的是参数一个右值，lambda传递进normalize的总是一个左值（形参x）
    return func(normalize(x));
};
// 上面对应的闭包类的函数调用操作符如下
class SomeCompilerGeneratedClassName {
public:
    template<typename T>
    auto operator()(T x) const {
        return func(normalize(x));
    }
};

// 实现这个lambda的正确方式是把x完美转发给函数normalize。
/*
如果一个左值实参传递给通用引用的形参，那形参类型就是左值引用
如果传递的是右值，则形参是右值引用。
所以在这个lambda中，可以通过检查形参x的类型来确定传递进来的实参是一个左值还是右值
decltype就可以实现这样的效果。
传递给lambda的是一个左值，decltype(x)就能产生一个左值引用
传递给lambda的是一个右值，decltype(x)就能产生右值引用

而在调用std::forward时，惯例决定了类型实参是左值引用时来表明要传进左值，类型实参是
非引用就表明要传进右值。
而在lambda中，如果x绑定一个右值，decltype(x)就会产生右值引用，而不是常规的非引用
x是左值，decltype(x)是左值引用，std::forward对x也是左值引用，std::forward<decltype(x)>(x)也是左值引用
x是右值，decltype(x)是右值引用，std::forward对x也是右值引用，std::forward<decltype(x)>(x)也是右值引用
虽然decltype(x)产生的类型传递给std::forward是非传统的，但是结果一致。
因此无论左值还是右值，把decltype(x)传递给std::forward都得到我们想要的结果
*/
auto f1 = [](auto&& x) {
    return func(normalize(std::forward<decltype(x)>(x)));
};
// 再加上6个点，实现lambda完美转发接受多个形参
auto f2 = [](auto&&... params) {
    return func(normalize(std::forward<decltype(params)>(params)...));
};
// 再看一下std::forward的C++14实现
template<typename T>
T&& forward1(std::remove_reference_t<T>& param) {
    return static_cast<T&&>(param);
}
// 如果想要完美转发一个Widget类型的右值时，会使用Widget类型（即非引用类型）来实例化std::forward
// 会产生下面的函数
Widget&& forward1(Widget& param) {
    return static_cast<Widget&&>(param);
}
// 若想要完美转发一个Widget类型的右值，但没有遵守规则将T指定为非引用类型，而是将T
// 指定为右值引用类型，会发生什么，std::forward类似下面的函数
// Widget&& && forward1(Widget& param) {
//     return static_cast<Widget&& &&>(param);
// }
// 应用引用折叠后（右值引用的右值引用变成单个右值引用）
// 完全相同。表明用右值引用类型和用非引用类型去初始化std::forward产生的结果相同
// Widget&& forward1(Widget& param) {
//     return static_cast<Widget&&>(param);
// }

int main() {
}