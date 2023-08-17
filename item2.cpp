/*
item2: 理解auto类型推导
    类似于item1中ParamType作用
    分为三个场景
    case 1. 类型说明符是一个指针或者引用，但不是通用引用
    case 2. 类型说明符是一个通用引用
    case 3. 类型说明符既不是指针也不是引用

    在C++14中允许auto用于函数返回值并会被推导，且lambda函数也允许在形参声明中使用auto。
    但是在这些情况下auto实际上使用模板类型推导的那一套规则在工作，而不是auto类型推导
*/
#include <iostream>
#include <vector>

template<typename T>
void f(T param) {

}

template<typename T>
void f1(std::initializer_list<T> initList) {

}

void someFunc(int, double) {
            
}

// C++14
auto createInt() {
    return 1;
}

// error C++14
// auto createInitList() {
//     return {1, 2, 3};
// }

int main() {
    {
        auto x = 27; // case 3
        const auto cx = x; // case 3
        const auto &rx = cx; // case 1

        auto&& uref1 = x; // x是int左值，所以uref1是int&
        auto&& uref2 = cx; // x是const int左值，所以uref1是const int&
        auto&& uref3 = 27; // x是int右值，所以uref1是int&&
    }
    {
        // 数组退化为指针，函数名退化为指针
        const char name[] = "R. N. Briggs";
        auto arr1 = name; // arr1类型为const char*
        auto& arr2 = name; // arr2类型为const char (&)[13]


        auto func1 = someFunc; // func1的类型为void (*)(int, double)
        auto& func2 = someFunc; // func2的类型为void (&)(int, double)
    }
    // auto类型推导与模板类型推导存在一些区别
    {
        // 声明一个带有初始值27的int，C++11右共有四种语法
        int x1 = 27; // C++98
        int x2(27); // C++98
        int x3 = {27}; // C++11
        int x4{27}; // C++11
        // 替换为auto
        auto x11 = 27; // int
        auto x21(27); // int
        auto x31 = {27}; // std::initializer_list<int> 
        auto x41{27}; // int

        // auto x5 = { 1, 2, 3.0}; // error 无法推导std::initializer_list<T>中的T

        // 对于花括号的处理是auto类型推导和模板类型推导唯一不同的地方。
        auto x = {11, 23, 9}; // x的类型是std::initializer_list<int>
        // f({11, 23, 9}); // error: 不能推导出T
        f1({11, 23, 9}); 
        // 所以，auto类型推导和模板类型推导的真正区别在于，auto类型推导假定花括号表示std::initializer_list
        // 而模板类型推导不会这样（确切的说是不知道怎么办）
    }
    {
        int x = createInt();
        std::cout << x << std::endl;
    }
    {
        std::vector<int> v;

        auto resetV = [&v](const auto& newValue) {
            v = {newValue}; // C++14
        };
        resetV(2);

        std::cout << v[0] << std::endl;
    }
    {
        // error
        // std::vector<int> v;

        // auto resetV = [&v](const auto& newValue) {
        //     v = newValue; // C++14
        // };
        // resetV({1, 2, 3});

    }
}