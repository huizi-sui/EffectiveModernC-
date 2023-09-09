/*
item41: 对于移动成本低且总是被拷贝的可拷贝形参，考虑按值传递

分析下面三种方式的拷贝和移动操作的开销。
1. 重载
无论传递左值还是右值，调用都会绑定到一个newName的引用上。从拷贝和移动操作方面看，
这个过程零开销。左值重载中，newName拷贝到Widget::names中，右值重载中，移动进去。
开销总结： 左值一次拷贝，右值一次移动。
2. 使用通用引用
同重载一样，调用也绑定到addName这个引用上，没有开销。
使用了std::forward，左值std::string实参会拷贝到Widget1::names中，右值std::string
实参移动进去。对std::string实参来说，开销同重载方式一样：
左值一次拷贝，右值一次移动。
3. 按值传递
无论传递左值还是右值，都需要构造newName形参。如果传递的是左值，需要拷贝的开销；如果传递
的是右值，需要移动的开销。在函数实现中，newName总是采用移动的方式到Widget2::names中。
开销总结：左值一次拷贝一次构造，右值两次移动

本item内容： 对于移动成本低且总是被拷贝的可拷贝形参，考虑按值传递
有四个原因：
1. 应该仅考虑使用传值方式。确实，只需要编写一个函数，只会在目标代码中生成一个函数，避免了
通用引用的种种问题。但是毕竟开销比那些替代方式更高，而且还存在一些其他开销。
2. 仅考虑对于可拷贝形参使用按值传递。不符合此条件的的形参必须有只可移动的类型（move-only types）（的数据成员），
因为函数总是会做副本（译注：指的是传值时形参总是实参的一个副本），
而如果它们不可拷贝，副本就必须通过移动构造函数创建。
（这样的句子就说明有一个术语来区分拷贝操作制作的副本，和移动操作制作的副本，是非常好的。）
回忆一下传值方案比“重载”方案的优势在于，仅有一个函数要写。但是对于只可移动类型，
没必要为左值实参提供重载，因为拷贝左值需要拷贝构造函数，只可移动类型的拷贝构造函数是禁用的。
那意味着只需要支持右值实参，“重载”方案只需要一个重载函数：接受右值引用的函数。
3. 按值传递应该仅考虑那些移动开销小的形参。 当移动的开销较低，额外的一次移动才能被开发者接受，
但是当移动的开销很大，执行不必要的移动就类似执行一个不必要的拷贝，而避免不必要的拷贝的
重要性就是最开始C++98规则中避免传值的原因。
4. 你应该只对总是被拷贝的形参考虑按值传递。如Widget5类的addName函数所示。
即使这个函数没有在names添加任何内容，也增加了构造和销毁newName的开销，而按引用传递就会避免这笔开销。


不像按引用传递，按值传递会受到切片问题的影响。
如果涉及一个函数，来处理这样的形参：基类或者任何其派生类，肯定不想声明一个那个类型的传值形参，
因为会“切掉”传入的任意派生类对象的派生类特征
class Widget {}; // 基类
class SpecialWidget : public Widget {}; // 派生类
void processWidget(Widget w); // 对任意类型的Widget的函数，包括派生类型
// 遇到对象切片问题
SpecialWidget sw;
processWidget(sw); // processWidget看到的是Widget, 不是SpecialWidget

。这样你就知道切片问题是C++98中默认按值传递名声不好的另一个原因（要在效率问题的原因之上）。有充分的理由来说明为什么你学习C++编程的第一件事就是避免用户自定义类型进行按值传递。

C++11没有从根本上改变C++98对于按值传递的智慧。
通常，按值传递仍然会带来你希望避免的性能下降，而且按值传递会导致切片问题。
C++11中新的功能是区分了左值和右值实参。
利用对于可拷贝类型的右值的移动语义，需要重载或者通用引用，尽管两者都有其缺陷。
对于特殊的场景，可拷贝且移动开销小的类型，传递给总是会拷贝他们的一个函数，并且切片也不需要考虑，
这时，按值传递就提供了一种简单的实现方式，效率接近传递引用的函数，但是避免了传引用方案的缺点。

请记住：

1. 对于可拷贝，移动开销低，而且无条件被拷贝的形参，
按值传递效率基本与按引用传递效率一致，而且易于实现，还生成更少的目标代码。
2. 通过构造拷贝形参可能比通过赋值拷贝形参开销大的多。
3. 按值传递会引起切片问题，所说不适合基类形参类型。
*/
#include <iostream>
#include <vector>
#include <memory>

/*
可行，但是麻烦
需要编写两个桉树来做本质相同的事。
两个函数声明，两个函数实现，两个函数的维护。
此外，目标代码中有两个函数——可能担心程序的空间占用。
两个函数都可能内联，可能会避免同时两个函数同时存在导致的代码膨胀问题。
但是一旦没有被内联，目标代码就会出现两个函数
*/
class Widget {
public:
    // 接受左值，拷贝它
    void addName(const std::string& newName) {
        names.push_back(newName);
    }
    // 接受右值，移动它
    void addName(std::string&& newName) {
        names.push_back(std::move(newName));
    }
private:
    std::vector<std::string> names;
};
/*
另一种方法：使addName函数称为具有通用引用的模板
减少了代码的维护工作，但是通用引用会导致其他复杂性

作为模板，addName的实现必须放置在头文件中。
在编译器展开的时候，可能会产生多个函数，因为不止为左值和右值实例化，也可能
为std::string和可转换为std::string的类型分别实例化多个函数。
同时还有些实参类型不能通过通用引用类型传递（参考item30）
*/
class Widget1 {
public:
    template<typename T>
    void addName(T&& newName) {
        // 接受左值和右值，拷贝左值，移动右值
        names.push_back(std::forward<T>(newName));
    }
private:
    std::vector<std::string> names;
};
/*
目的：编写addName的方法，可以左值拷贝，右值移动，只用处理一个函数，且避免通用引用。
std::move典型的应用场景是用在右值引用
在这里
1， newName是与调用者传进来的对象完全独立的一个对象，所以改变newName不会影响到调用者
2. 这就是newName的最后一次使用，所以移动它不会影响函数内此后的其他代码。

在C++98中，开销很大。无论调用者传入什么，形参newName都是由拷贝构造出来。
在C++11中，只有在左值实参情况下，addName被拷贝构造出来；对于右值，它会被移动构造。
*/
class Widget2 {
public:
    // 接受左值和右值，移动它
    void addName(std::string newName) {
        names.push_back(std::move(newName));
    }
private:
    std::vector<std::string> names;
};

class Widget3 {
public:
    void setPtr(std::unique_ptr<std::string>&& ptr) {
        p = std::move(ptr);
    }
private:
    // 只可移动的类型
    std::unique_ptr<std::string> p;
};
class Widget4 {
public:
    void setPtr(std::unique_ptr<std::string> ptr) {
        p = std::move(ptr);
    }
private:
    // 只可移动的类型
    std::unique_ptr<std::string> p;
};
class Widget5 {
public:
    void addName(std::string newName) {
        int minLen = 1;
        int maxLen = 10;
        if(newName.length() >= minLen && newName.length() <= maxLen) {
            names.push_back(newName);
        }
    }
private:
    std::vector<std::string> names;
};

/*
即使编写的函数对可拷贝类型执行无条件的复制，且这个类型移动开销小，有时候也
不适合按值传递。
因为函数拷贝一个形参存在两种方式：一种是通过构造函数（拷贝构造或移动构造），
还有一种是赋值（拷贝赋值或移动赋值）。
addName使用构造函数，它的形参newName传递给vector::push_back，在这个函数内部，
newName是通过拷贝构造在vector末尾创建一个新元素。
对于使用构造函数拷贝形参的函数，之前分析已经给出最终结论：按值传递对于左值和右值
均增加了一次移动操作的开销。

当形参通过赋值操作进行拷贝，分析更加复杂。
*/
class Passward {
public:
    // 形参传值，text使用构造
    explicit Passward(std::string pwd) : text(std::move(pwd)) {

    }
    // 形参传值，text使用赋值
    void changeTo(std::string newPwd) {
        text = std::move(newPwd);
    }
private:
    std::string text;
};
class Passward1 {
public:
    // 形参传值，text使用构造
    explicit Passward1(std::string pwd) : text(std::move(pwd)) {

    }
    // 形参传值，text使用赋值
    void changeTo(std::string& newPwd) {
        // 若text.capacity() >= newPwd.size()，可能重用text的内存
        text = newPwd;
    }
private:
    std::string text;
};

// 对象切片问题
class Base {
public:
    int baseVar;
    void show() {
        std::cout << "Base: " << baseVar << std::endl;
    }
};

class Derived: public Base {
public:
    int derivedVar;
    void show() {
        std::cout << "Derived: " << baseVar << " " << derivedVar << std::endl;
    }
};

int main() {
    {
        Widget2 w;
        std::string name("Bart");
        w.addName(name); // 使用左值调用addName
        w.addName(name + "Jenne"); // 使用右值调用addName
    }
    {
        Widget3 w;
        // 从std::make_unique返回的右值std::unique_ptr<std::string>
        // 通过右值引用被传给了setPtr，然后移动到数据成员p中。
        // 整体开销就是一次移动。
        w.setPtr(std::make_unique<std::string>("Modern C++"));
        Widget4 w1;
        // 先移动构造ptr形参，后ptr再移动赋值到数据成员p
        // 整体开销就是两次移动
        w1.setPtr(std::make_unique<std::string>("Modern C++"));
    }
    {
        std::string initPwd("Supercaligfhkahgldjalfafqawwwww");
        Passward p(initPwd);

        std::string newPassword = "Beware the Jabberwock";
        // 关心的是changeTo使用赋值来拷贝构造形参newPwd，可能导致函数的按值传递实现方案开销大大增加
        p.changeTo(newPassword);
        /*
        传递给changeTo的实参是一个左值，所以newPwd形参被构造时，std::string的拷贝构造函数被调用
        函数会分配新的内存给新密码。newPwd会移动赋值到text，这会导致text本来持有的内存被释放。
        所以changeTo存在两次动态内存管理的操作：一次是为新密码创建内存，一次是销毁旧密码的内存
        */
        Passward1 p1(initPwd);
        /*
        旧密码比新密码长度更长，所以不需要分配新内存，销毁旧内存的操作。
        若使用重载法，有可能两次动态内存管理操作得以避免。
        这种情况下，按值传递的开销包括了内存分配和内存销毁——可能会比std::string的移动操作高出几个数量级。
        有趣的是，如果旧密码短于新密码，在赋值过程中就不可避免要销毁、分配内存，
        这种情况，按值传递跟按引用传递的效率是一样的。
        因此，基于赋值的形参拷贝操作开销取决于具体的实参的值，
        这种分析适用于在动态分配内存中存值的形参类型。不是所有类型都满足，
        但是很多——包括std::string和std::vector——是这样。
        */
        p1.changeTo(newPassword);
    }
    {
        // 对象切片问题
        Derived derivedObj;
        Base baseObj = derivedObj; // 对象切片，只赋值了Base部分
        baseObj.show();
    }
}