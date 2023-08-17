/*
item12： 使用override声明重写函数

请记住：
1. 为重写函数加上override
2. 成员函数引用限定让我们可以区别对待左值对象和右值对象（即*this)

在C++面向对象的世界里，涉及的概念有类，继承，虚函数。
这个世界最基本的概念是派生类的虚函数重写基类同名函数。

重写函数必须满足下面的要求
1. 基类函数必须是virtual
2. 基类和派生类函数名必须完全一样，除非是析构函数
3. 基类和派生类函数形参类型必须完全一样
4. 基类和派生类函数常量性必须完全一样
5. 基类和派生类函数的返回值和异常说明必须兼容
除了这些C++98就存在的约束外，C++11又添加了一个：
6. 函数的引用限定符必须完全一样。它可以限定成员函数只能用于左值或者右值，
成员函数不需要virtual也能使用它们

如果基类的虚函数有引用限定符，派生类的重写就必须具有相同的引用限定符。
如果没有，那么新声明的函数还是属于派生类，但是不会重写父类的任何函数

C++11引入两个上下文关键字， override和final
向虚函数添加final可以防止派生类重写。final也能用于类，这时这个类不能用作基类
特点： 它们是保留的，只是位于特定上下文才被视为关键字。
对于override，只在成员函数声明结尾处才被视为关键字。
class Warning {
public:
    void override(); // C++98和C++11都合法，且含义相同
};
*/
#include <iostream>
#include <memory>
#include <vector>

class Base {
public:
    virtual void doWork() {} // 基类虚函数
};

class Derived : public Base {
public:
    virtual void doWork() { // 重写Base::doWork()，这里virtual可以省略
    
    }
};

class Widget {
public:
    void doWork() & { // 只有*this为左值的时候才能被调用
        std::cout << "doWork() &" << std::endl;
    }
    void doWork() && { // 只有*this为右值的时候才能被调用
        std::cout << "doWork() &&" << std::endl;
    }
};

// 工厂函数，返回右值
Widget makeWidget() {
    Widget w;
    return w;
}

// 下面Base1和Derived1完全合法
// 但是没有任何虚函数重写——没有一个派生类函数联系到基类函数
class Base1 {
public:
    virtual void mf1() const {}
    virtual void mf2(int x) {}
    virtual void mf3() & {}
    virtual void mf4() const {}
};

class Derived1 : public Base1 {
public:
    virtual void mf1() {} // 缺少函数的常量const声明
    virtual void mf2(unsigned int x) {} // 形参问题
    virtual void mf3() && {} // 引用限定问题
    void mf4() const {} // 基类没有声明为virtual虚函数
};

// C++11提供override显式地指定一个派生类函数是基类版本的重写
class Derived2 : public Base1 {
public:
    virtual void mf1() const override {}
    virtual void mf2(int x) override {}
    virtual void mf3() & override {}
    void mf4() const override {}
};

class Warning {
public:
    void override(){} // C++98和C++11都合法，且含义相同
};

// 如果想写一个函数只接受左值实参，可以声明一个non-const左值引用对象
void doSomething(Widget& w) {
    std::cout << "doSomething(Widget& w)" << std::endl;
}
// 一个函数只接受右值实参
void doSomething(Widget&& w) {
    std::cout << "doSomething(Widget&& w)" << std::endl;
}
/*
成员函数的引用限定可以很容易的区分一个成员函数被哪个对象（即*this）调用。
它和在成员函数声明尾部添加一个const很相似，暗示了调用这个成员函数的对象（即*this）是const的
*/
class Widget1 {
public:
    using DataType = std::vector<double>;

    DataType& data() {
        return values;
    }
private:
    DataType values;
};

// 工厂函数，返回右值
Widget1 makeWidget1() {
    Widget1 w1;
    return w1;
}

class Widget2 {
public:
    using DataType = std::vector<double>;
    DataType& data() & { // 对于左值Widget2，返回左值
        return values;
    }
    DataType data() && { // 对于右值Widget2，返回右值
        return std::move(values);
    }
private:
    DataType values;
};

Widget2 makeWidget2() {
    Widget2 w2;
    return w2;
}

int main() {
    {
        // 创建积累指针指向派生类对象
        std::unique_ptr<Base> upb = std::make_unique<Derived>();
        // 通过积累指针调用doWork，实际上是派生类的doWork函数被调用
        upb->doWork();
    }
    {
        Widget w; // 普通对象， 左值
        w.doWork(); // 调用被左值引用限定修饰的函数版本

        makeWidget().doWork(); // 调用被右值引用限定的函数版本
    }
    {
        Widget w;
        doSomething(w);
        doSomething(makeWidget());
    }
    {
        Widget1 w1;
        auto values1 = w1.data(); // 拷贝w1.values到values1
        /*
        Widget1::data函数返回值是一个左值引用（准确的说是std::vector<double>&）
        因为左值引用是左值，所以values1是从左值初始化的。因此values1由w1.values拷贝构造而得
        */
       auto values2 = makeWidget1().data(); // 拷贝Widget1里面的值到values2
       /*
       再次强调，Widget1::data返回的是左值引用，且左值引用是左值
       所以values2得从Widget1里的values拷贝构造。
       这一次，Widget1是makeWidget1返回的临时对象，即右值，所以将其中的std::vector进行拷贝纯属浪费
       最好是移动，但是因为data返回左值引用，C++的规则要求编译器不得不生成一个拷贝。
       所以需要的是指明当data被右值Widget1对象调用的时候结果也应该是一个右值。修改成Widget2类
       */
    }
    {
        Widget2 w2;
        // 调用左值重载版本的Widget::data, 拷贝构造values1
        auto values1 = w2.data();
        // 调用右值重载版本的Widget::data, 移动构造values2
        auto values2 = makeWidget2().data();
    }
}