#include <iostream>

/*
item18: 让接口容易被正确使用，不易被误用。
1. 好的接口很容易被正确使用，不易被误用。应在所有接口中努力达成这些性质
2. “促进正确使用”的办法包括接口的一致性，以及与内置类型的行为兼容。
3. “阻止误用”的办法包括建立新类型、限制类型上的操作，束缚对象值，以及消除客户的资源管理责任。

// 3个参数类型相同的函数容易造成误用
Data::Data(int month, int day, int year) {}
// 通过适当定义新的类型加以限制，降低误用的可能性
Data::Data(const Month& m, const Day& d, const Year& y) {}

4. 尽量使用智能指针，避免跨DLL的new和delete，使用智能指针自定义删除器来解除互斥锁
*/

/*
item19：设计class犹如设计type
几乎在设计每一个class时，都要面对如下问题：

新type对象应该如何被创建和销毁？
这会影响到类中构造函数、析构函数、内存分配和释放函数(operator new, operator new[],
operator delete, operator delete[])的设计。

对象的初始化和赋值该有什么样的差别？
这会影响到构造函数和拷贝赋值运算之间行为的差异。

新type对象如果被按值传递，意味着什么？
这会影响到拷贝构造函数的实现。

什么是新type的合法值？
你的类中的成员函数必须对类中成员变量的值进行检查，如果不合法就要尽快解决或明确地抛出异常。

你的新type需要配合某个继承图系么？
你的类是否受到基类设计的束缚，是否拥有该覆写的虚函数，是否允许被继承(若不想被继承，应该声明为final)

什么样的运算符和函数对此新type是合理的？
这会影响到你将为你的类声明哪些函数和重载哪些运算符

什么样的标准函数应该被驳回？
这会影响到你将哪些标准函数声明为= delete

谁该取用新type的成员？
为未声明接口提供效率、异常安全性以及资源运用上的保证，并在实现代码中加上相应的约束条件。

你的新 type 有多么一般化？
如果你想要一系列新 type 家族，应该优先考虑模板类
*/

/*
item20：宁以常引用传参替换按值传参
当使用按值传参时，程序会调用对象的拷贝构造函数构建一个在函数内作用的
局部对象，这个过程的开销可能会较为昂贵。
对于任何用户自定义类型，使用按常引用传参是较为推荐的
*/
class Student {

};
bool ValidateStudent(const Student& s) {

}
// 因为没有任何对象被创建，这种传参方式不会调用任何构造函数或析构函数，
// 所以效率比按值传参高得多。
// 使用按引用传参也可以避免对象切片问题
class Window {
public:
    std::string GetName() const {
        return "xxx";
    }
    virtual void Display() const {

    }
};
class WindowWithScrollBars : public Window {
public:
    virtual void Display() const override {

    }
};
// 按值传递，会发生对象切片
// 此处在传参时，调用了基类Window的拷贝构造函数而非派生类的拷贝构造函数，
// 因此在函数种使用的是一个Window对象，调用虚函数时也只能调用到基类的虚函数Window::Display。
void PrintNameAndDisplay(Window w) {
    std::cout << w.GetName();
    w.Display();
}
// 按引用传递不会创建新对象，可以避免这个问题
void PrintNameAndDisplay(const Window& w) { // 参数不会被切片
    std::cout << w.GetName();
    w.Display();
}
// 对于内置类型、STL的迭代器和函数对象，按值传递比较合适。

/*
item21: 必须返回对象时，别妄想返回其引用

返回一个指向函数内部局部变量的引用是严重的错误，因为局部变量在离开
函数时就被销毁了，除此之外，返回一个指向局部静态变量的引用也是不被推荐的。

尽管返回对象会调用拷贝构造函数产生开销，但这开销比起出错而言微不足道。
*/

/*
item22：将成员变量声明为private

出于对封装性的考虑，应该尽可能地隐藏类中地成员变量，并通过对外
暴露函数接口实现对成员变量地访问
*/
class AccessLevels {
public:
    int GetReadOnly() const {
        return readOnly;
    }
    void SetReadWrite(int value) {
        readWrite = value;
    }
    int GetReadWrite() const {
        return readWrite;
    }
    void SetWriteOnly(int value) {
        writeOnly = value;
    }
private:
    int noAccess;
    int readOnly;
    int readWrite;
    int writeOnly;
};
/*
通常为成员变量提供getter和setter函数，就能避免
用户做出写入只读变量或读取只写变量这样不被允许地操作。

将成员变量隐藏在函数接口地背后，可以为“所有可能地实现”提供弹性。
例如这可使得在成员变量被读或写时轻松通知其他对象，可以验证类的
约束条件以及函数的提前和事后状态，可以在多线程环境中执行同步控制....

protected和public一样，都不该被优先考虑。假设有一个public成员变量，
最终取消了它，那么所有使用它的客户代码都将被破环；假设有一个protected
成员变量，最终取消了它，那么所有使用它的派生类都将被破环。

综上，在类中应当将成员变量优先声明为private
*/

/*
item23：宁以非成员、非友元函数替代成员函数
*/
class WebBrowser {
public:
    void ClearCache() {

    }
    void ClearHistory() {

    }

    void RemoveCookies() {

    }
};
// 想要一次性调用这三个函数，那么需要额外提供一个新的函数
void ClearEverything(WebBrowser& wb) {
    wb.ClearCache();
    wb.ClearHistory();
    wb.RemoveCookies();
}
/*
虽然成员函数和非成员函数都可以完成目标，但是建议使用非成员函数。
这是为了遵守一个原则：越少的代码可以访问数据，数据的封装性就越强。
这里的ClearEverything函数仅仅是调用了WebBrowser的三个public成员
函数，而并没有使用到WebBrowser内部的private成员，因此没有必要让其
也拥有访问类中private成员的能力。


这个原则对友元函数也是相同的，因为友元函数和成员函数拥有相同的权力，
所以在能使用非成员函数完成任务的情况下，就不要使用友元函数和成员函数。

若觉得一个全局函数并不自然，也可以考虑将ClearEverything函数放在工具类中
充当静态成员函数，或与WebBrowser放在同一个命名空间中。
*/
namespace WebBrowserStuff {
    class WebBrowser {

    };
    void ClearEverything(WebBrowser& wb) {
        // ...
    }
};

/*
item24: 若所有参数皆需类型转换，请为此采用非成员函数

现有一个Rational类，并且它可以和int隐式转换
*/
class Rational {
public:
    Rational(int numerator = 0, int denominator = 1) {

    }
    // 重载乘法运算符
    const Rational operator*(const Rational& rhs) const {

    }
};
void test() {
    Rational oneEight(1, 8);
    Rational oneHalf(1, 2);
    // Rational result = oneHalf / oneEight;
    Rational result;
    result = oneHalf * 2;
    // result = 2 * oneHalf; // 报错
    // 将上面写出函数形式，错误的原因就很清楚了
    result = oneHalf.operator*(2);
    // result = 2.operator*(oneHalf); // error
    // 在调用operator*时，int类型的变量会隐式转换为Rational对象
    // 因此用Rational对象乘以int对象是合法的，但反过来则不是如此。
}
// 为了避免这个错误，应当将运算符重载放在外面，作为非成员函数
const Rational operator*(const Rational& lhs, const Rational& rhs) {

}

/*
item25：考虑写一个不抛异常的swap函数

由于std::swap函数在C++11后改为了用std::move实现，因此几乎没有了
性能缺陷，也不再有像原书所说的为自定义类型去自己实现的必要。但
原书中透露的思想值得一学。

如果想为自定义类型实现自己的swap，可以考虑使用模板全特化。
*/
class WidgetImpl {

};
class Widget {
public:
    void swap(Widget& other) {
        using std::swap;
        std::cout << "类内自定义实现的swap" << std::endl;
        swap(pImpl, other.pImpl);
    }
private:
    WidgetImpl* pImpl;
};

namespace std {
    template<>
    void swap<Widget>(Widget& a, Widget& b) {
        std::cout << "特化的swap" << std::endl;
        a.swap(b);
    }
}
// 由于外部函数并不能直接访问Widget的private成员变量，因此
// 先在类内定义一个public成员函数，再由std::swap去调用这个成员函数。

// 若Widget和WidgetImpl是类模板，就复杂了，因为C++不支持函数模板偏特化，
// 所以只能使用重载的方式
/*
namespace std {
    template<typename T>
    void swap(Widget<T>& a, Widget<T>& b) {
        a.swap(b);
    }
}

但这是被STL禁止的，因为这是再试图向STL中添加新的内容，因此只能退而求其次，
在其他命名空间中定义新的swap函数
*/
namespace WidgetStuff {
    template<typename T>
    class Widget {
    public:
        void swap(Widget<T>& other) {
            using std::swap;
            std::cout << "Widget<T>中的swap" << std::endl;
            swap(t, other.t);
        }
    private:
        T* t;
    };
    template<typename T> 
    void swap(Widget<T>& a, Widget<T>& b) {
        std::cout << "WidgetStuff中的swap" << std::endl;
        a.swap(b);
    }
}


int main() {
    Widget a, b;
    std::swap(a, b);

    WidgetStuff::Widget<int> w1, w2;
    // 希望使用自定义的swap函数重载版本，因此不能使用std::swap
    // 它会强制使用STL中的swap函数，需要改写
    using std::swap;
    using namespace WidgetStuff;
    swap(w1, w2);
    // 这样，C++名称查找法则能保证我们优先使用的是自定义的swap函数而非STL中的swap函数
    /*
    C++名称查找法则：编译器会从使用名字的地方开始向上查找，由内向外查找各级作用域(命名空间)
    直到全局作用域(命名空间)，找到同名的声明即停止，若最终没有找到则报错。

    函数匹配优先级：普通函数 > 特化函数 > 模板函数
    */
}