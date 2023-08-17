/*
lambda表达式
*/

#include <iostream>
#include <memory>

class sizeComp {
public:
    sizeComp(size_t s) : sz(s) {

    }

    bool operator()(const std::string &s) const {
        return s.size() > sz;
    }
private:
    size_t &sz;
};

int a = 10; // 全局变量

class A {
public:
    void print() {
        std::cout << "class A print() x = " << x << std::endl;
    }
    void test() {
        auto foo = [this]() {
            x = 5;
            print();
        };
        foo();
    }
private:
    int x;
};

int main() {
    size_t sz = 10;
    // lambda表达式被编译器编译为类，如上面类的定义
    auto sizeComp = [&sz](const std::string &s) {
        return s.size() > sz;
    };

    auto func_mutable = [sz]() mutable {
        // 默认是const，不能修改捕获值
        // 使用mutable，可修改
        sz = 10;
    };

    // lambda表达式只需捕获非静态成员变量
    // 全局变量和静态变量无需捕获，本来就是可以见到的
    static int c = 20;
    auto func1 = [sz]() {
        std::cout << "sz = " << sz << std::endl;
        std::cout << "a = " << a << std::endl;
        std::cout << "c = " << c << std::endl;
    };

    auto func2 = [sz]() {
        std::cout << "sz = " << sz << std::endl;
    };

    sz = 20;
    // lambda表达式捕获发生在定义时，而非调用时，因此打印sz为10，而不是20
    func2();
    // 无法被拷贝
    auto important = std::make_unique<int>(1);
    auto add = [v1 = 1, v2 = sz](int a, int b) -> int {
        return v1 + v2 + a + b;
    };
    //C++14, 允许lambda表达式进行广义捕获。允许传递右值
    auto add1 = [v1 = 1, v2 = std::move(important)](int a, int b) -> int {
        return v1 + (*v2) + a + b;
    };

    A a;
    a.test();
}