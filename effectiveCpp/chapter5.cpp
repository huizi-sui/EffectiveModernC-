#include <iostream>
#include <memory>
#include <vector>
#include <mutex>
/*
item26: 尽可能延后变量定义式出现的时间

当变量定义出现时，程序需要承受其构造成本；当变量离开其作用域时，
程序需要承受其析构成本。
因此，避免不必要的变量定义，以及延后变量定义式直到你确实需要它。

延后变量定义式还有一个意义，即“默认构造+赋值”效率低于“直接构造”
// 效率低
std::string encrypted;
encrypted = password;
// 效率高
std::string encrypted(password);

对于循环中变量的定义，一般由两种做法
1. 定义于循环外，在循环中赋值
这种做法的开销：1个构造函数+1个析构函数+n个赋值函数
2. 定义于循环内
这种做法的开销：n个构造函数+n个析构函数

由于做法1会将变量的作用域扩大，因此除非直到该变量的赋值成本比
“构造+析构”成本低，或者对这段程序的效率要求非常高，否则建议使用做法2
*/
class Widget {
public:
    Widget() {}
    Widget(int i) {
        std::cout << "构造函数" << std::endl;
    }
    Widget(const Widget& w) {
        std::cout << "拷贝构造" << std::endl;
    }
    Widget& operator=(const Widget& w) {
        std::cout << "拷贝赋值运算符" << std:: endl;
        return *this;
    }
    ~Widget() {
        std::cout << "析构函数" << std::endl;
    }
};
void test(int n) {
    Widget w;
    Widget w1;
    for(int i = 0; i < n; ++i) {
        w = w1; 
    }
    for(int i = 0; i < n; ++i) {
        Widget w(i);
    }
}

/*
item27：少做转型动作
C式转型
(T)expression
T(expression)
C++式转型
const_cast<T>(expression)
用于常量性转除，这也是唯一一个有这个能力的C++式转型
dynamic_cast<T>(expression)
用于安全地向下转型，这也是唯一一个C式转型无法代替地转型操作。它会
执行对继承体系地检查，因此会带来额外的开销。只有拥有虚函数的基类指针
能进行dynamic_cast
reinterpret_cast<T>(expression)
用于在任意两个类型间进行低级转型，执行该转型可能会带来风险，也可能不具备移植性。
static_cast<T>(expression)
用于进行强制类型转换，也是最常用的转型操作，可以将内置数据类型互相转换，也可以将void*
和typed指针，基类指针和派生类指针互相转换。

尽量在C++程序中使用C++式转型，因为C++式转型操作功能更明确，可以避免不必要的错误。
唯一使用C式转型的时机可能是在调用explicit构造函数时
*/
class Widget1 {
public:
    explicit Widget1(int size) {

    }
};
void DoSomeWork(const Widget1& w) {

}
void test1() {
    DoSomeWork(Widget1(2));
    // 等价于
    DoSomeWork(static_cast<Widget1>(2));
}
// 需要注意的是，转型并非什么都没有做，而是可能会更改数据的底层表示，
// 或者为指针附加偏移值，这和具体平台有关，因此不要妄图去揣测转型后对象的具体布局方式

// 避免对*this进行转型
class Window {
public:
    virtual void OnResize() {

    }
};
class SpecialWindow : public Window {
public:
    virtual void OnResize() override {
        // 这里试图通过转型*this来调用基类的虚函数，这是严重错误的，
        // 这样做会得到一个新的Window副本并在该副本上调用函数，
        // 而非在原本的对象上调用函数。(对象切片问题)
        // static_cast<Window>(*this).OnResize();
        Window::OnResize(); // 正确做法
    }
};
// 当想知道一个基类指针是否指向一个派生类对象时，需要使用dynamic_cast，
// 如果不满足，则会产生错误。但是对于继承体系的检查可能是非常慢的，所以在
// 注重效率的程序中应当避免使用dynamic_cast，改用static_cast或别的代替方法。

/*
item28：避免返回handles指向对象的内部部分
*/
struct Point {
    float x;
    float y;
};
struct RectData {
    Point ulhc;
    Point lrhc;
};
class Rectangle {
public:
    // 通过const成员函数返回一个指向成员变量的引用，
    // 这使得成员变量可以在外部被修改，违反了logical constness原则。
    // 绝对不应该令成员函数返回一个指针指向“访问级别较低”的成员函数
    // 改成常引用可以避免对成员变量的修改
    // 但是这样会带来一个称作dangling handles(空悬句柄)的问题，
    // 当对象不复存在时，将无法通过引用获取到返回的数据
    // Point& UpperLeft() const {
    const Point& UpperLeft() const {
        return pData->lrhc;
    }
    // Point& LowerRight() const {
    const Point& LowerRight() const {
        return pData->ulhc;
    }

    // 采用最保守的做法，返回一个成员变量的副本
    Point UpperLeft1() const {
        return pData->ulhc;
    }
    Point LowerRight1() const {
        return pData->lrhc;
    }
private:
    std::shared_ptr<RectData> pData;
};

/*
Item29: 为“异常安全”而努力是值得的
异常安全函数提供以下三个保证之一：
基本承诺：
如果异常被抛出，程序内的任何事物仍然报错在有效状态下，没有任何对象或数据结构会因此
败坏，所有对象都处于一种内部前后一致的状态，然而程序的真实状态是不可知的，也就是说
客户需要额外检查程序处于哪种状态并做出对应的处理。

强烈保证：
如果异常被抛出，程序状态完全不改变，换句话说，程序会回复到“调用函数之前”的状态。

不抛掷(nothrow)保证：
承若绝不抛出异常，因为程序总是能完成原先承诺的功能。作用于内置类型身上的所有操作都
提供nothrow保证。

原书中实现nothrow的方法是throw(), 不过这套异常规范在C++11中已经被弃用了，取而代之
的是noexcept关键字

int DoSomething() noexcept;
注意使用noexcept并不代表函数绝对不会抛出异常，而是在抛出异常时，将代表出现严重错误，
会有意想不到的函数被调用(可以通过set_unexcepted设置)，接着程序会直接崩溃。

当异常被抛出时，带有异常安全性的函数会：
1. 不泄露任何资源
2. 不允许数据败坏
*/
class Image {
public:
    Image(std::vector<uint8_t>& imgSrc) {

    }
    void reset(std::shared_ptr<Image> pImage) {

    }
};
class PrettyMenu {
public:
    void ChangeBackground(std::vector<uint8_t>& imgSrc);
    void ChangeBackground1(std::vector<uint8_t>& imgSrc);
private:
    std::mutex mutex;
    Image* bgImage;
    int imageChanges;
};
void PrettyMenu::ChangeBackground(std::vector<uint8_t>& imgSrc) {
    mutex.lock();
    delete bgImage;
    ++imageChanges;
    bgImage = new Image(imgSrc);
    mutex.unlock();
}
// 这个函数不满足我们所说的具有异常安全性的任何一个条件，若在函数中抛出异常，
// mutex会发生资源泄露，bgImage和imageCache也会发生数据败坏。
/*
资源泄漏：当异常在mutex.lock()和mutex.unlock()之间抛出时，mutex可能会被永远
锁定，导致其他线程无法获得锁，从而导致死锁。因为锁资源没有被释放，导致程序无法正常执行。

数据败坏：如果异常在delete bgImage和bgImage = new Image(imgSrc)之间抛出。
因为bgImage指向的内存已经被释放，但没有新的Image对象来代替它。如果后续代码尝试访问
bgImage，它将指向已经释放的内存，这可能导致未定义行为。
*/

// 通过以对象管理资源，使用智能指针和调换代码顺序，将其变成一个具有强烈保证的异常安全函数
void PrettyMenu::ChangeBackground1(std::vector<uint8_t>& imgSrc) {
    mutex.lock();
    bgImage->reset(std::make_shared<Image>(imgSrc));
    ++imageChanges;
    
}
// 另一个常用于提供强烈保证的方法是copy and swap，为你打算修改的对象做出一个副本，
// 对副本进行修改，并在所有修改都成功执行后，用一个不会抛出异常的swap方法将原件和副本交换
struct PMImpl {
    std::shared_ptr<Image> bgImage;
    int imageChanges;
};
class PrettyMenu1 {
public:
    void ChangeBackground(std::vector<uint8_t>& imgSrc);
private:
    std::mutex mtx;
    std::shared_ptr<PMImpl> pImpl;
};
void PrettyMenu1::ChangeBackground(std::vector<uint8_t>& imgSrc) {
    mtx.lock();

    auto pNew = std::make_shared<PMImpl>(*pImpl); // 获取副本
    pNew->bgImage->reset(std::make_shared<Image>(imgSrc));
    ++pNew->imageChanges;

    std::swap(pImpl, pNew);
}
// 当一个函数调用其他函数时，函数提供的“异常安全保证”通常最高只等于
// 其所调用的各个函数的“异常安全保证”中的最弱者。
// 强烈保证并非永远都是可实现的，特别是当函数在操控非局部对象时，
// 这时就只能退而求其次选择不那么美好的基本承诺，并将该决定写入文档，
// 让其他人维护时不至于毫无心理准备

/*
item30：透彻了解inlining的里里外外
将函数声明为内联一共两种方法，一种是为其显式指定inline关键字，
另一种是直接将成员函数额定义式写在类中
*/
class Person {
public:
    int Age() const { // 隐式声明为inline
        return theAge;
    }
private:
    int theAge;
};
/*
在inline诞生之初，被当做是一种对编译器的优化建议，
即将“对此函数的每一个调用”都以函数本体替换之。但在编译器具体实现中，
该行为完全被优化等级所控制，与函数是否内联无关。

在现在的C++标准中，inline作为优化建议的含义已经被完全抛弃，取而代之的是
“允许函数在不同编译单元中多重定义”，使得可以在头文件直接给出函数的实现。

C++17中，引入一个新的inline用法，使静态成员可以在类中直接定义
class Person {
public:
private:
    static inline int theAge = 10;
};
*/

/*
item31: 将文件间的编译依存关系降至最低
C++坚持将类的实现细节放置于类的定义式中，这意味着，即使你只改变类的实现
而不改变类的接口，在构建程序中依然需要重新编译。这个问题的根源出在编译器
必须在编译期间知道对象的大小，如果看不到类的定义式，就没有办法为对象分配内存。
也就是说，C++并没有把“将接口从实现中分离”这件事做得很好。

用“声明的依存性”替换“定义的依存性”
将对象实现细目隐藏于一个指针背后，称为pimpl idiom。
将原来的一个类分解为两个类，一个只提供接口，另一个负责实现该接口，称为句柄类(handle class)
*/
// person.hpp 负责声明类
class PersonImpl;
class Person1 {
public:
    Person1();
    void Print();
private:
    std::shared_ptr<PersonImpl> pImpl;
};
// person.cpp负责实现类
class PersonImpl {
public:
    int data{0};
};
Person1::Person1() {
    pImpl = std::make_shared<PersonImpl>();
}
void Person1::Print() {
    std::cout << pImpl->data;
}
// 这样，假设需要修改Person的private成员，只需要修改PersonImpl
// 中的内容，而PersonImpl的具体实现是被隐藏起来的，对它的任何修改
// 都不会使得Person客户端重新编译，真正实现了“类的接口和实现分离”

/*
如果使用对象引用或对象指针可以完成任务，就不要使用对象本身：
可以只靠一个类型声明式就定义出指向该类型的引用和指针；但如果定义某类型
的对象，就需要用到该类型的定义式。

如果能够，尽量以类声明式替换类定义式：
当在声明一个函数而它用到某个类时，不需要该类的定义；当触及到该函数的定义式后，
就必须也知道类的定义
*/
class Data; // 类的声明式
Data Today();
void ClearAppointments(Data d); // 此处并不需要知道类的定义

/*
为声明式和定义式提供不同的头文件：
为了避免频繁地添加声明，应该为所有要用的类声明提供一个头文件，这种做法对template也适用
#include "datefwd.h" // 这个头文件内声明class Date
Date today();
void ClearAppointments(Data d);

另一个方法就使将句柄类定义为抽象基类，称为接口类(interface class)
*/
class Person2 {
public:
    virtual ~Person2() {

    }
    virtual void Print() {

    }
    // 为了将Person对象实际创建出来，一般采用工厂模式。
    // 可以在类中塞入一个静态成员函数Create用于创建对象
    static std::shared_ptr<Person2> Create() {
        return std::make_shared<RealPerson>();
    }
};
class RealPerson : public Person2 {
public:
    RealPerson() {

    }
    virtual ~RealPerson() {

    }
    void Print() override {

    }
private:
    int data{0};
};
/*
毫无疑问的是，句柄类和接口类都需要额外的开销：
句柄类需要通过pimpl取得数据，增加一层间接访问、指针大小和动态分配内存
带来的开销；而接口类会增加存储虚表指针和实现虚函数跳转带来的开销。

而当这些开销过于重大以至于类之间的耦合度在相比之下不成为关键时，
就以具象类替代句柄类和接口类。
*/

int main() {
    test(2);
}