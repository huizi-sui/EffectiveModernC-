/*
    item1: 理解模板类型推导
    模板函数：
    template<typename T>
    void f(ParamType param);
    调用：
    f(expr); // 使用表达式调用f
    编译期间，编译器使用expr进行两个类型推导：1个是针对T，另一个是针对ParamType的。
    这两个类型通常不同，因为ParamType包含一些修饰，比如const和引用修饰符。
    
    希望T与传递进函数的实参是相同类型，也就是，T为expr类型，但实际上，T的类型推导不仅取决于expr类型，
    也取决于ParamType类型，分为三种情况。
    1. ParamType是一个指针或者引用，但不是通用引用
    2. ParamType是一个通用引用
    3. ParamType既不是指针也不是引用
*/
#include <iostream>

/*
1， ParamType是一个指针或者引用，但不是通用引用
类型推导：
1. 如果expr的类型是一个引用，忽略引用部分
2. 然后expr的类型于ParamType进行模式匹配来决定T
*/
template<typename T>
void f(T& param) { // param是一个引用
    std::cout << param << std::endl;
}

template<typename T>
void f1(const T& param) { // param是一个常量引用

}

template<typename T>
void f2(T* param) { // param是一个指针

}

/*
 2. ParamType是一个通用引用
 如果expr是左值，T和paramType都会被推到为左值引用。
 (1. 这是模板类型推导中唯一一种T被推导为引用的情况；
  2，虽然ParamType被声明为右值引用类型，但是最后推导的结果是左值引用)
 如果expr是右值，就使用正常的推导规则（情况1）
*/
template<typename T>
void f3(T&& param) { // param现在是一个通用引用类型

}

/*
3. ParamType既不是指针也不是引用
此时通过传值方式处理。
这意味着无论传递什么param都会成为它的一份拷贝——一个完整的新对象。
param成为一个新对象这一行为会影响T如何从expr中推导出结果。
1. 如果expr的类型是一个引用，忽略这个引用部分
2. 如果忽略expr的引用性后，expr是一个const，则再忽略const，如果它是volatile，也忽略volatile
*/
template<typename T>
void f4(T param) { // 以传值方式处理param
    std::cout << param << std::endl;
}

// 数组形参没有名字，因为只关注数组大小
//将一个函数声明为constexpr使得结果在编译期间可用
template<typename T, std::size_t N>
constexpr std::size_t arraySize(T (&)[N]) noexcept {
    return N;
}

void someFunc(int a, double b) {
    std::cout << "a = " << a << ", b = " << b << std::endl;
}

template<typename T>
void f5(T param) {
    std::cout << param << std::endl;
    param(2, 3);
}

template<typename T>
void f6(T& param) {
    std::cout << param << std::endl;
    param(4, 5);
}

class Widget {

};

int main() {
    {
        // 1. ParamType是一个指针或者引用，但不是通用引用
        int x = 27; // x是int
        const int cx = x; // cx是const int
        const int& rx = x; // rx是指向作为const int的x的引用
        f(x); // T是int， param的类型是int&
        f(cx); // T是const int, param的类型是const int&
        f(rx); // T是const int, param的类型是const int&

        f1(x); // T是int, param的类型是const int&
        f1(cx); // T是int, param的类型是const int&
        f1(rx); // T是int, param的类型是const int&

        const int *px = &x;
        f2(&x); // T是int, param的类型是int*
        f2(px); // T是const int, param的类型是const int*
    }
    {
        // 2. ParamType是一个通用引用
        int x = 27; // x是int
        const int cx = x; // cx是const int
        const int& rx = x; // rx是指向作为const int的x的引用
        f3(x); // x是左值，T是int&, param也是int&
        f3(cx); // cx是左值，所以T是const int&, param也是const int&
        f3(rx); // rx是左值，所以T是const int&, param也是const int&
        f3(27); // 27是右值，所以T是int， param也是int&&
    }
    {
        // 3. ParamType既不是指针也不是引用
        int x = 27; // x是int
        const int cx = x; // cx是const int
        const int& rx = x; // rx是指向作为const int的x的引用
        f4(x); // T和param的类型都是int
        // param是一个完全独立于cx和rx的对象——是cx或rx的一个拷贝
        // 具有常量性的cx和rx不可修改不代表param也是一样。
        // 因为expr不可修改并不意味着它的拷贝也不能被修改
        f4(cx); // T和param的类型都是int
        f4(rx); // T和param的类型都是int

        const char* const ptr = "Fun with pointers"; // ptr是一个常量指针，指向常量对象
        // 右边const表示ptr是一个const，ptr不能被修改为指向其他地址，也不能被设置为nullptr。
        // 左边const表示ptr指向一个字符串，这个字符串是const，因此该字符串不能被修改
        // 根据第三条规则，ptr自身的const会被省略，因此param是const char*，即一个可变指针指向const字符串
        f4(ptr);

    }
    {
        const char name[] = "J. P. Briggs";
        const char* ptrToName = name; // 数组退化为指针
        f4(name); // name是一个数组，但是T被推导为const char*
        // 虽然函数不能声明形参为真正的数组（会退化为指针），但是可以接受指向数组的引用
        f(name); // T被推导为真正的数据，且包括了数组的大小，这里T被推导为const char[13], param则为const char (&)[13]

        int keyVals[] = {1, 3, 7, 9, 11, 22, 25};
        std::cout << arraySize(keyVals) << std::endl;
    }
    {
        // 在C++中，不只是数组会退化为指针，函数类型也会退化为一个函数指针
        f5(someFunc); // param被推导为指向函数的指针， 类型是void(*)(int, double)
        f6(someFunc); // param被推导为指向函数的引用，类型是void(&)(int, double)
        
    }

    {
        const Widget* w = new Widget();
        f1(w);

        int x = 10;
        const int& y = x;
        int const& z = x;
        std::cout << typeid(y).name() << std::endl;
        std::cout << typeid(z).name() << std::endl;
    }

}