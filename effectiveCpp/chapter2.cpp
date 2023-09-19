#include <iostream>
#include <algorithm>
/*
item5：了解C++默默编写并调用哪些函数
*/
// C++中的空类并不是真正意义上的空类，编译器会为它预留以下内容：
class Empty {
public:
    Empty() {} // 默认构造函数（没有任何构造函数时）
    Empty(const Empty&) {} // 拷贝构造函数
    Empty(Empty&&) {} // 移动构造函数(C++11)
    ~Empty() {} // 析构函数

    Empty& operator=(const Empty&) { return *this; } // 拷贝赋值运算符
    Empty& operator=(Empty&&) { return *this; } // 移动赋值运算符(C++11)
};
// 唯有当这些函数被调用时，它们才会真正被编译器创建出来
void createEmpty() {
    Empty e1; // 默认构造函数 & 析构函数
    Empty e2(e1); // 拷贝构造函数
    Empty e3 = std::move(e1); // 移动构造函数

    e2 = e1; // 拷贝赋值运算符
    e3 = std::move(e1); // 移动赋值运算符
}
// 需要注意的是，拷贝赋值运算符只有在允许存在时才会自动创建
class NamedObject {
public:
    NamedObject(std::string& x) : nameValue(x) {
    }
private:
    std::string& nameValue;
};
// 在该类中，有一个string引用类型，然而引用无法指向不同的对象，
// 因此编译器会拒绝为该类创建一个默认的拷贝赋值运算符。
// 初此之外，以下情况也会导致拷贝赋值运算符不会自动创建
// 1. 类中含有const成员
// 2. 基类中含有private的拷贝赋值运算符。

/*
item6： 若不想使用编译器自动生成的函数，就该明确拒绝
原书中使用的做法是将不想使用的函数声明为private，
但是在C++11后有了更好的做法，使用delete说明符
*/
class Uncopyable {
public:
    Uncopyable(const Uncopyable&) = delete;
    Uncopyable& operator=(const Uncopyable&) = delete;
};

/*
item7: 为多态基类声明虚析构函数
当派生类对象经由一个基类指针被删除，而该基类指针带有一个非虚析构函数，
其结果是未定义的，可能会无法完全销毁派生类的成员，造成内存泄露。
消除这个问题的方法就是对基类使用虚析构函数
*/
class Base {
public:
    Base() {}
    virtual ~Base() {}
};
// 如果不想让一个类成为基类，那么在类中声明虚函数是一个坏主意，
// 因为额外存储的虚表指针会使类的体积变大。

// 只要基类的析构函数是虚函数，那么派生类的析构函数不论是否用
// virtual关键字声明，都自动成为虚析构函数。

// 虚析构函数的运作方式是，最深层派生的那个类的析构函数最先被调用，
// 然后是其上的基类的析构函数被依次调用。

// 如果想将基类作为抽象类使用，但手头上有没有别的虚函数，那么将
// 它的析构函数设为纯虚函数是一个不错的想法。
class Base1 {
public:
    virtual ~Base1() = 0; // 纯虚函数
};
// 即使析构函数写为纯虚函数，也需要函数体
Base1::~Base1() { }
// 或者以下写法也被允许
/*
class Base {
public:
    virtual ~Base() = 0 {}
};
*/
class Derival : public Base1 {
public:
    ~Derival() {
        std::cout << "~Derival()" << std::endl;
    }
};

/*
item8: 别让异常逃离析构函数
在析构函数中吐出异常并不被禁止，但为了程序的可靠性，应当极力避免这种行为。
*/

// 为了实现RAII，通常将对象的销毁方法封装在析构函数中
class DBConnection {
public:
    void close() {} // 假设可能会抛出异常
};
class DBConn {
public:
    ~DBConn() {
        db.close(); // 该函数可能会抛出异常
    }
private:
    DBConnection db;
};
// 但这样就需要在析构函数中完成对异常的处理。

// 1. 杀死程序
/*
DBConn::~DBConn() {
    try {
        db.close();
    } catch(std::Exception& e) {
        // 记录运行日志，方便调试
        std::abort();
    }
}
*/
// 2. 直接吞下异常不做处理，但是这种做法不被建议
// 3. 重新设计接口，将异常的处理交给客户端完成。
class DBConn1 {
public:
    void close() {
        db.close();
        closed = true;
    }
    ~DBConn1() {
        if(!closed) {
            try {
                db.close();
            } catch(std::exception& e) {
                // 处理异常
            }
        }
    }
private:
    DBConnection db;
    bool closed;
};
// 在这个新设计的接口中，提供close函数供客户手动调用，
// 这样客户也可以根据自己的意愿处理异常；若客户忘记手动调用，
// 析构函数才会自动调用close函数。

// 当一个操作可能抛出需要客户处理的异常时，将其暴露哎普通函数
// 而非析构函数中是一个更好的选择。

/*
item9： 绝不在构造和析构过程中调用虚函数
在创建派生类对象时，基类的构造函数永远会早于派生类的构造函数被调用，
而基类的析构函数永远会晚于派生类的析构函数被调用。

在派生类对象的基类构造和析构期间，对象的类型是基类而非派生类，
因此此时调用虚函数会被编译器解析至基类的虚函数版本，通常不会得到
我们想要的结果。

间接调用虚函数是一个比较难以发现的危险行为，需要尽量避免：
*/
class Transaction {
public:
    Transaction() {
        Init();
    }
    virtual void LogTransaction() const = 0;
private:
    void Init() {
        //...
        LogTransaction(); // 此处间接调用虚函数
    }
};
// 如果想要基类在构造时就得知派生类的构造信息，推荐的做法是
// 在派生类的构造函数中将必要的信息向上传递给基类的构造函数
class Transaction1 {
public:
    explicit Transaction1(const std::string& logInfo);
    void LogTransaction(const std::string& logInfo) const {

    }
};

Transaction1::Transaction1(const std::string& logInfo) {
    LogTransaction(logInfo); // 更改为了非虚函数调用
}

class BuyTransaction : public Transaction1 {
public:
    // 将信息传递给基类的构造函数
    BuyTransaction() : Transaction1(CreateLogString()) {

    }

private:
    static std::string CreateLogString() {
        return "hello";
    }
};
// 注意这里CreateLogString是一个静态成员函数，这很重要，
// 因为静态成员函数可以确保不会使用未完成初始化的成员变量

/*
item10：令operator=返回一个指向*this的指针
虽然不强制执行该条款，但为了实现连锁赋值，大部分时候应该这样做
*/
class Widget {
public:
    Widget& operator+=(const Widget& rhs) {
        // ...
        return *this;
    }
    Widget& operator=(int rhs) {
        //...
        return *this;
    }
};

/*
item11: 在operator=中处理自我赋值
自我赋值是合法的操作，但在一些情况下可能会导致意外的错误，
例如在复制堆上的资源时
*/
class Widget1 {
public:
    Widget1& operator+=(const Widget1& rhs) {
        delete s; // 删除当前持有的资源
        s = new std::string(*rhs.s); // 复制传入的资源
        return *this;
    }
private:
    std::string* s;
};
// 但是若rhs和*this指向相同的对象，就会导致访问到已删除的数据
// 最简单的方法是在执行后续语句前先进行证同测试
class Widget2 {
public:
    Widget2& operator=(const Widget2& rhs) {
        if(this == &rhs) { // 若是自我赋值，则不做任何事
            return *this;
        }
        delete s; // 删除当前持有的资源
        s = new std::string(*rhs.s); // 复制传入的资源
        return *this;
    }
private:
    std::string* s;
};
// 另一个常见的做法是只关注异常安全性，而不关注是否自我赋值
class Widget3 {
public:
    Widget3& operator=(const Widget3& rhs) {
        std::string* pOrigin = s; // 先记住原来的指针
        s = new std::string(*rhs.s); // 复制传入的资源
        delete s; // 删除当前持有的资源
        return *this;
    }
private:
    std::string* s;
};
// 仅仅适当安排语句的顺序，就可以做到使整个过程具有异常安全性
// 还有一种取巧的方法是使用copy and swap技术，这种技术利用了栈
// 空间会自动释放的特性，这样就可以通过析构函数来实现资源的释放
// class Widget4 {
// public:
//     Widget4& operator=(const Widget4& rhs) {
//         Widget4 temp(rhs);
//         std::swap(*this, temp);
//         return *this;
//     }
//     // 还可以按值传参，自动调用构造函数
//     Widget4& operator=(Widget4 rhs) {
//         std::swap(*this, rhs);
//         return *this;
//     }
// private:
//     std::string* s;
// };

/*
item12: 复制对象时勿忘其每一个成分

当你决定手动实现拷贝构造函数和拷贝赋值运算符时，忘记复制任何
一个成员都可能会导致意外的错误。

当使用继承时，继承自基类的成员往往容易忘记在派生类中完成复制，如果你的
基类拥有拷贝构造函数和拷贝赋值运算符，应当记得调用它们
*/
class Customer {
public:
    Customer(const Customer& rhs) {

    }
    Customer& operator=(const Customer& rhs) {

    }
};
class PriorityCustomer : public Customer {
public:
    PriorityCustomer(const PriorityCustomer& rhs);
    PriorityCustomer& operator=(const PriorityCustomer& rhs);
private: 
    int priority;
};

PriorityCustomer::PriorityCustomer(const PriorityCustomer& rhs) 
    : Customer(rhs), // 调用基类的拷贝构造函数
      priority(rhs.priority) {

}

PriorityCustomer& PriorityCustomer::operator=(const PriorityCustomer& rhs) {
    Customer::operator=(rhs); // 调用基类的拷贝赋值运算符
    priority = rhs.priority;
    return *this;
}
// 不要尝试在拷贝构造函数中调用拷贝赋值运算符，
// 或在拷贝赋值运算符的实现中调用拷贝构造函数，
// 一个在初始化时，一个在初始化后，它们的功用是不同的。

int main() {
    Derival d;
}