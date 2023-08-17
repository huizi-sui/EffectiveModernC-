/*
item7: 区别使用()和{}创建对象

*/
#include <iostream>
#include <vector>
#include <atomic>

class Widget {
public:
    Widget(){
        std::cout << "Widget()" << std::endl;
    }
    Widget(int _x) {
        std::cout << "Widget(int)" << std::endl;
    }
    Widget(int i, bool b) {
        std::cout << "Widget(int, bool)" << std::endl;
    }
    Widget(int i, double d) {
        std::cout << "Widget(int, double)" << std::endl;
    }
};

class Widget1 {
public:
    Widget1(int i, bool b) {
        std::cout << "Widget1(int, bool)" << std::endl;
    }
    Widget1(int i, double d) {
        std::cout << "Widget1(int, double)" << std::endl;
    }
    Widget1(std::initializer_list<long double> il) {
        std::cout << "Widget1(std::initializer_list)" << std::endl;
    }
    operator float() const { // 转换为float
        std::cout << "operator float()" << std::endl;
    }
};

class Widget2 {
public:
    Widget2(int i, bool b) {
        std::cout << "Widget2(int, bool)" << std::endl;
    }
    Widget2(int i, double d) {
        std::cout << "Widget2(int, double)" << std::endl;
    }
    Widget2(std::initializer_list<bool> il) {
        std::cout << "Widget2(std::initializer_list)" << std::endl;
    }
};

class Widget3 {
public:
    Widget3(int i, bool b) {
        std::cout << "Widget3(int, bool)" << std::endl;
    }
    Widget3(int i, double d) {
        std::cout << "Widget3(int, double)" << std::endl;
    }
    Widget3(std::initializer_list<std::string> il) {
        std::cout << "Widget3(std::initializer_list)" << std::endl;
    }
};

class Widget4 {
public:
    Widget4() {
        std::cout << "Widget4()" << std::endl;
    }
    Widget4(std::initializer_list<int> il) {
        std::cout << "Widget4(std::initializer_list)" << std::endl;
    }
};

// T是要创建的对象类型， Ts是要使用的实参类型
template<typename T, typename... Ts> 
void doSomeWork(Ts&& ...params) {
    // 两种方式实现
    // 如果T是std::vector<int>，则不清楚是创建一个什么vector
    // 可能是size为2，值为10， 20的vector
    // 也可能是size为10，值都为20的vector
    T localObject(std::forward<Ts>(params)...);
    T localObject1{std::forward<Ts>(params)...};
}

int main() {
    {
        int x(0); // 使用圆括号初始化
        int y = 0; // 使用=初始化
        int z{0}; // 使用花括号初始化
        int z1 = {0}; // 与int z{0}一样
    }

    Widget w1; // 调用默认构造函数
    Widget w2 = w1; // 不是赋值语句，调用拷贝构造函数
    w1 = w2; // 赋值运算，调用拷贝赋值运算符
    // 无法创建并初始化一个存放一些特殊值的STL容器（如1， 3， 5）

    // C++11使用统一初始化来整合这些混乱且不适合所有场景的初始化语法
    // 所谓统一初始化是指在任何涉及初始化的地方都使用单一的初始化语法。
    std::vector<int> v{1, 3, 5};

    // 不可拷贝的对象（如std::atomic）可以使用花括号初始化或者圆括号初始化
    // 但是不能使用=初始化
    std::atomic<int> ai1{0};
    std::atomic<int> ai2{0};
    // std::atomic<int> ai3 = 0; // error


    // {}表达式不允许内置类型间隐式的变窄转换
    double x, y, z;
    // int sum1{x + y + z}; // error
    // ()与=不检查是否转换为变窄转换，因为由于历史遗留问题它们必须要兼容老旧代码
    int sum2(x + y + z);
    int sum3 = x + y + z;

    {
        /*
        C++规定任何可以被解析为一个声明的东西必须解析为声明，有很大的副作用
        想要创建一个使用默认构造函数构造的对象，却不小心变成了函数声明
        */
       Widget w1(10); // 使用实参10调用Widget的一个构造函数
       Widget w2(); // 尝试调用Widget的无参构造函数，却变成了函数声明
       // 函数声明中形参列表不能带花括号
       Widget w3{}; // 调用没有参数的构造函数对象 
       int x = 10;
    }
    // {}初始化也存在不少问题
    {
        Widget w1(10, true); // 调用Widget(int, bool)
        Widget w2{10, true}; // 调用Widget(int, bool)
        Widget w3(10, 5.0); // 调用Widget(int, double)
        Widget w4{10, 5.0}; // 调用Widget(int, double)
    }
    std::cout << "=============\n";
    {
        // 如果有一个或多个构造函数的声明包含std::initializer_list形参
        // 使用{}初始化语法更倾向于带std::initializer_list的构造函数
        // 甚至普通构造函数和移动构造函数都会被带std::initializer_list的构造函数劫持
        Widget1 w1(10, true); // 调用Widget(int, bool)
        Widget1 w2{10, true}; // 调用Widget(std::initializer_list), 转换为long_double
        Widget1 w3(10, 5.0); // 调用Widget(int, double)
        Widget1 w4{10, 5.0}; // 调用Widget(std::initializer_list)， 转换为long_double
        Widget1 w5(w4); // 使用圆括号，调用拷贝构造函数
        Widget1 w6{w4}; // 调用std::initializer_list构造函数，w4转换为float，float转换为double
        Widget1 w7(std::move(w4)); // 调用移动构造函数
        Widget1 w8{std::move(w4)}; // 调用std::initializer_list构造函数

        // 编译器会忽略前两两个构造函数（其中第二个构造函数是所有实参类型的最佳匹配）
        // 然后尝试调用std::initializer_list<bool>构造函数
        // 将int和double转换为bool，产生变窄转换，编译失败
        // Widget2 w{10, 5.0}; // error     
    }
    {
        // 当没有办法把括号初始化实参的类型转化为std::initializer_list
        // 编译器才会回到正常函数决议流程中
        std::cout << "==========" << std::endl;
        Widget3 w1(10, true); // 调用Widget(int, bool)
        Widget3 w2{10, true}; // 调用Widget(int, bool)
        Widget3 w3(10, 5.0); // 调用Widget(int, double)
        Widget3 w4{10, 5.0}; // 调用Widget(int, double)  
    }
    std::cout << "===========" << std::endl;
    {
        // 默认构造函数和std::initializer_list构造函数都存在时
        // 使用{}初始化是空集时，调用默认构造函数
        Widget4 w1; // 调用默认构造函数
        Widget4 w2{}; // 调用默认构造函数
        Widget4 w3(); // 声明了一个函数
        // 若想用空std::initializer来调用std::initializer_list构造函数
        Widget4 w4({});
        Widget4 w5{{}};
    }
    {
        /*
        std::vertor有一个非std::initializer_list构造函数允许指定容器初始大小，
        以及使用一个值填满容器。
        也有一个std::initializer_list构造函数允许使用花括号里面的值初始化容器
        */
       // 使用非std::initializer_list构造函数，创建一个包含10个元素，都为20的vector
       std::vector<int> v1(10, 20); 
       // 使用std::initializer_list构造函数，创建包含10，20两个元素的vector
       std::vector<int> v2{10, 20};
    }
    {
        std::vector<int> v;
        // 产生问题
        doSomeWork<std::vector<int>>(10, 20);
    }
}