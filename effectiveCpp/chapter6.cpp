#include <iostream>
#include <functional>
#include <list>
/*
item32：确定你的public继承塑模处is-a关系

“public继承”意味着is-a，所谓is-a，就是指适用于基类身上的每一件事情
一定也适用于继承类身上，因为我们可以认为每一个派生类对象也都是一个基类对象。

这看似很自然，但是在自然语言表述时，往往会产生歧义。
*/
class Brid {
public:
    virtual void Fly() {
        std::cout << "Brid::Fly" << std::endl;
    }
};
class Penguin : public Brid {

};
/*
Penguin类会获得来自Bird类的飞行方法，这就造成了误解，因为企鹅
恰恰是不会飞的鸟类。

一种解决方法：当调用Penguin类的Fly函数时，抛出一个运行时异常，
但这种做法不够直观。

另一种解决方法是适用双继承，区分会飞和不会飞的鸟类。
若要处理鸟类的多种不同属性时，双继承模式就不太管用了，
因此说程序设计没有银弹。(即在软件开发中没有一种单一的、通用的
、适用于所有情况的解决方案和技术。)

另一个常见的例子是用Square类继承自Rectangle类，从几何学的角度来
讲这很自然，然而正方形的长宽是相等的，矩形却不是这样，
因此Square类和Rectangle类也无法满足严格的is-a关系。
*/
class Bird1 {

};
class FlyingBird : public Bird1 {
public:
    virtual void Fly() {

    }
};
class Penguin1 : public Bird1 {

};

/*
item33： 避免遮掩继承而来的名称

当我们在派生类中使用到一个名字时，编译器会优先查找派生类覆盖的作用域，
如果没找到，再去查找基类的作用域，最后查找全局作用域。
*/
class Base {
public:
    void mf() {

    }
    void mf(double) {

    }
};
class Derived : public Base {
public:
    void mf() {

    }
};
// 这样会导致派生类无法使用来自基类的重载函数，因为
// 派生类中的名称mf掩盖了来自基类的名称mf。
// 对于名称掩盖问题的一种方法是使用using关键字。
class Derived1 : public Base {
public:
    using Base::mf;
};
// using关键字会将基类中所有使用到名称mf的函数全部包含在派生类中，
// 包括其重载版本。

// 有时不想要一个函数的全部版本，只想要单一版本(特别是在private继承时)，
// 可以考虑使用转发函数(forwarding function)
class Base1 {
public:
    virtual void mf() {

    }
    virtual void mf(double) {

    }
};
class Derived2 : public Base1 {
public:
    virtual void mf() {
        Base1::mf();
    }
};
/*
item34: 区分接口继承和实现继承

1. 接口继承和实现继承不一样。在public继承下，派生类总是继承基类的接口
2. 声明一个纯虚函数的目的是为了让派生类只继承函数接口。
3. 声明简朴的非纯虚函数的目的，是让派生类继承该函数的接口和缺省实现
4. 声明非虚函数的目的，是为了令派生类继承函数的接口以及一份强制性实现
通常而言，不会为纯虚函数提供具体实现，然而这样做是被允许的，并且用于替代
简朴的非纯虚函数，提供更平常更安全的缺省。

用非纯虚函数提供缺省的默认实现：
这是最简朴的做法，但是存在问题：由于不强制对虚函数的覆写，
在定义新的派生类时可能会忘记进行覆写，导致错误的使用了缺省实现。
*/
class Airplane {
public:
    virtual void Fly() {
        // 缺省实现
    }
};
class Model : public Airplane {

};
/*
使用纯虚函数并提供默认实现
*/
class Airplane1 {
public:
    virtual void Fly() = 0;
protected:
    void DefaultFly() {
        // 缺省实现
    }
};
class Model1 : public Airplane1 {
public:
    virtual void Fly() override {
        DefaultFly();
    }
};
// 上述写法等价于
class Airplane2 {
public:
    virtual void Fly() = 0;
};
void Airplane2::Fly() {
    // 缺省实现
}
class Model2 : public Airplane2 {
public:
    virtual void Fly() override {
        Airplane2::Fly();
    }
};
/*
item35: 考虑虚函数以外的其他选择

藉由非虚接口手法实现template method：
非虚接口(non-virtual interface，NVI)设计手法的
核心就是用一个非虚函数作为wrapper，将虚函数隐藏在封装之下.

优点：在wrapper中做一些前置和后置工作，确保得以在一个虚函数被调用
之前设定好适当场景，并在调用结束之后清理场景。若让客户直接调用虚函数，
就没有任何办法可以做这些事。

NVI手法允许派生类重新定义虚函数，从而赋予它们“如何实现机能”的控制能力，
但基类保留诉说“函数何时被调用”的权利。

在NVI手法中虚函数除了可以是private，也可以是protected，例如要求在
派生类的虚函数实现内调用其基类的对应虚函数时，就必须得这么做。
*/
class GameCharacter {
public:
    int HealthValue() const {
        // 做一些前置工作
        int retVal = DoHealthValue();
        // 做一些后置工作
        return retVal;
    }
private:
    virtual int DoHealthValue() const {
        // 缺省算法
        int value = 10;
        return value;
    }
};
/*
藉由函数指针实现Strategy模式：
同一人物类型的不同实体可以有不同的健康计算函数，并且该计算函数
可以在运行期变更。

这间接表明健康计算函数不再是GameCharacter1继承体系内的成员函数，
它也无权使用非public成员。为了填补这个缺陷，唯一的做法是弱化类的
封装，引入友元或提供public访问函数。
*/
class GameCharacter1;
int DefaultHealthCalc(const GameCharacter1&); // 缺省算法
class GameCharacter1 {
public:
    // 定义函数指针类型
    using HealthCalcFunc = int(*)(const GameCharacter1&);
    explicit GameCharacter1(HealthCalcFunc hcf = DefaultHealthCalc) : healthFunc(hcf) {

    }
    int HealthValue() const {
        return healthFunc(*this);
    }
private:
    HealthCalcFunc healthFunc;
};
/*
藉由std::function完成Strategy模式
std::function是C++11引入的函数包装器，使用它能
提供比函数指针更强的灵活性
*/
class GameCharacter2;
int DefaultHealthCalc2(const GameCharacter2&); // 缺省算法
class GameCharacter2 {
public:
    // 定义函数包装器类型
    using HealthCalcFunc = std::function<int(const GameCharacter2&)>;
    explicit GameCharacter2(HealthCalcFunc hcf = DefaultHealthCalc2)
        : healthFunc(hcf) {

    }
    int HealthValue() const {
        return healthFunc(*this);
    }
private:
    HealthCalcFunc healthFunc;
};
// 看起来没有多大的改变，但当我们需要时，std::function就能展示出惊人的弹性
// 使用返回值不同的函数
short CalcHealth(const GameCharacter2&) {
    return 1;
}
// 使用函数对象(仿函数)
struct HealthCalculator {
    int operator()(const GameCharacter2&) const {
        int value = 10;
        return value;
    }
};
// 使用某个成员函数
class GameLevel {
public:
    float Health(const GameCharacter2&) const {
        return 1.0f;
    }
};
void test() {
    GameCharacter2 chara1(CalcHealth);
    GameCharacter2 chara2(HealthCalculator());
    GameLevel currentLevel;
    GameCharacter2 chara3(std::bind(&GameLevel::Health, currentLevel, std::placeholders::_1));
}
/*
古典的Strategy模式：
在古典的Strategy模式中，并非直接利用函数指针或包装器调用函数，
而是内含一个指针指向来自HealthCalcFunc继承体系的对象.
好处：足够容易辨认，想要添加新的计算函数也只需要为HealthCalcFunc基类
添加一个派生类即可。
*/
class GameCharacter3;
class HealthCalcFunc3 {
public:
    virtual int Calc(const GameCharacter3& gc) const {
        int value = 10;
        return value;
    }
};
HealthCalcFunc3 defaultHealthCalc;
class GameCharacter3 {
public:
    explicit GameCharacter3(HealthCalcFunc3* phcf = &defaultHealthCalc)
        : pHealthCalc(phcf) {

    }
    int HealthValue() const {
        return pHealthCalc->Calc(*this);
    }
private:
    HealthCalcFunc3* pHealthCalc;
};

/*
item36：绝不重新定义继承而来的非虚函数

非虚函数和虚函数具有本质上的不同：非虚函数执行的是静态绑定(statically bound,
又称前期绑定，early binding)，由对象类型本身(称之静态类型)决定要调用的函数；
而虚函数执行的是动态绑定(dynamically bound, 又称后期绑定，late binding)，
决定因素不在对象本身，而在于“指向该对象之指针”当初的声明类型(称为动态类型)

public继承意味着is-a关系，而在基类中声明一个非虚函数将会为该类建立起一种不变性
(invariant)，凌驾其特异性(specialization)。而若在派生类中重新定义该非虚函数，
则会使人开始质疑是否该使用public继承的形式；如果必须使用，则又打破了基类“不变性
凌驾特异性”的性质，就此产生了设计上的矛盾。
*/

/*
item37：绝不重新定义继承而来的缺省参数值

条款36已经否定了重新定义非虚函数的可能性，这里只讨论带有缺省参数值的虚函数。

虚函数是动态绑定而来，意思是调用一个虚函数时，究竟调用哪一份函数实现代码，
取决于发出调用的那个对象的动态类型。但与之不同的是，缺省参数值却是静态绑定，
意思是你可能会在“调用一个定义于派生类的虚函数”的同时，却使用基类为它指定的
缺省参数值。
*/
class Shape {
public:
    enum class ShapeColor {
        Red, Green, Blue
    };
    virtual void Draw(ShapeColor color = ShapeColor::Red) const = 0;
};
class Rectangle : public Shape {
public:
    virtual void Draw(ShapeColor color = ShapeColor::Green) const {
        if(color == ShapeColor::Red) {
            std::cout << "red";
        } else if(color == ShapeColor::Green) {
            std::cout << "green";
        } else {
            std::cout << "blue";
        }
        std::cout << std::endl;
    }
};
class Circle : public Shape {
public:
    virtual void Draw(ShapeColor color) const {
        if(color == ShapeColor::Red) {
            std::cout << "red";
        } else if(color == ShapeColor::Green) {
            std::cout << "green";
        } else {
            std::cout << "blue";
        }
        std::cout << std::endl;
    }
};
void test1() {
    Shape* pr = new Rectangle;
    Shape* pc = new Circle;

    pr->Draw(Shape::ShapeColor::Green); // green
    pr->Draw(); // red
    pc->Draw(); // red
}
// 这迫使我们在指定虚函数时使用相同的缺省参数值
// 为了避免不必要的麻烦和错误，可以考虑item35中列出
// 的虚函数的替代设计，例如NVI手法：
class Shape1 {
public:
    enum class ShapeColor {
        Red, Green, Blue
    };
    void Draw(ShapeColor color = ShapeColor::Red) const {
        DoDraw(color);
    }
private:
    virtual void DoDraw(ShapeColor color) const = 0;
};
class Rectangle1 : public Shape1 {
public:
private:
    /*
    派生类可以重写基类的private虚拟函数。
    虚拟函数的可访问性(即是否是public、protected、private)对于派生类
    是否能够重写这个函数没有影响。这是因为派生类只需要知道基类中有一个
    虚拟函数，然后可以选择性的重写它。

    需要注意，虽然派生类重写基类的私有虚拟函数，但在派生类外无法访问这个函数。
    必须在类的内部函数调用。
    */
    virtual void DoDraw(ShapeColor color) const {

    }
};

/*
item38：通过复合塑模出has-a或“根据某物实现出”
所谓复合(composition)，指的是某种类型的对象内含它种类型的对象。
复合通常意味着has-a或根据某物实现出(is-implemented-in-terms-of)的关系，
当复合发生于应用域(application domain)内的对象之间，表现出has-a
的关系，当它发生于实现域(implementation domain)内则表现出
“根据某物实现出”的关系
*/
// has-a示例
class Address {

};
class PhoneNumber {

};
class Person {
public:
private:
    std::string name; // 合成成分物
    Address address; // 合成成分物
    PhoneNumber voiceNumber; // 合成成分物
    PhoneNumber faxNumber; // 合成成分物
};
// 根据某物实现出示例
template<class T>
class Set {
public:
    bool member(const T& item) const;
    void insert(const T& item);
    void remove(const T& item);
    std::size_t size() const;
private:
    std::list<T> rep; // 用来表述Set的数据
};

/*
item39: 明智而审慎地使用private继承
private继承地特点：
1. 如果类之间是private继承关系，那么编译器不会自动将一个派生类对象转换为一个基类对象
2. 由private继承来的所有成员，在派生类种都会变为private属性，换句话说，private继承
只继承实现，不继承接口。

private继承的意义是“根据某物实现出”，private继承与复合具有相同的意义，
绝大部分private继承的使用场合都可以被“public继承+复合”完美解决。
*/
class Timer {
public:
    explicit Timer(int tickFrequency);
    virtual void OnTick() const;
};
class Widget : private Timer {
private:
    virtual void OnTick() const;
};
// 替换为：
class Widget1 {
private:
    class WidgetTimer : public Timer {
    public:
        virtual void OnTick() const;
    };
    WidgetTimer timer;
};
/*
使用后者比前者好的原因：
1. private继承无法阻止派生类重新定义虚函数，但若使用public
继承定义WidgetTimer类并复合在Widget类中，就能防止在Widget
类种重新定义虚函数。
2. 可以仅提供WidgetTimer类的声明，并将WidgetTimer类的具体定义
移至实现文件中，从而降低Widget的编译依存性。

然而private继承也有适用的情况： 空白基类最优化(empty base optimization, EBO)
*/
class Empty {

};
class HoldsAnInt {
private:
    int x;
    Empty e;
};
// 一个没有非静态成员变量、虚函数的累，看似不需要任何存储空间，
// 但实际上C++规定凡是独立对象都必须有非零大小，
// 因此此处sizof(HoldsAnInt)必然大于sizeof(int)，通常会多出1字节，
// 但有时考虑到内存对齐之类的问题，可能会多出更多的空间。
// 使用private继承可以避免产生额外存储空间
// 替换代码为：
class HoldsAnInt1 : private Empty {
private: 
    int x;
};

/*
item40: 明智而审慎的使用多重继承

多重继承是一个可能会造成很多歧义和误解的设计。
程序可能从一个以上的基类继承相同名称，那会导致较多的歧义机会
*/
class BorrowableItem {
public:
    void CheckOut();
};
class ElectronicGadget {
public:
    void CheckOut() const;
};
class MP3Player : public BorrowableItem, public ElectronicGadget {

};
void test2() {
    MP3Player mp;
    // mp.CheckOut(); // 不明确
    mp.BorrowableItem::CheckOut(); // 使用BorrowableItem::CheckOut
}
// 在使用多继承时，可能会遇到要命的“钻石型继承(菱形继承)”
class File {

};
class InputFile : public File {

};
class OutputFile : public File {

};
class IOFile : public InputFile, public OutputFile {

};
// 这时候必须面对这样一个问题：是否打算让基类内的成员变量经由每一条路径
// 被复制(在IOFile种将会有两个独立的File子对象)。
// 如果不想要这样，应当使用虚继承，指出其愿意共享基类。(只有一个共享的File子对象)
class File1 {

};
class InputFile1 : virtual public File1 {

};
class OutputFile1 : virtual public File1 {

};
class IOFile1 : public InputFile1, public OutputFile1 {

};
/*
由于虚继承会在派生类种额外存储信息来确认成员来自哪个基类，
虚继承通常会付出更多空间和速度的代价，并且由于虚继承的初始化责任是
由继承体系种最底层的派生类负责，导致虚基类必须认知其虚基类并且承担
虚基类的初始化责任。因此应当遵循以下两个建议：
1. 非必要不使用虚继承
2. 如果必须使用虚继承，尽可能避免在虚基类中放置数据。
多重继承可用于结合public继承和private继承，public继承用于提供接口，
private继承用于提供实现
*/
// IPerson类之处要实现的接口
class IPerson {
public:
    virtual ~IPerson();
    virtual std::string Name() const = 0;
    virtual std::string BirthDate() const = 0;
};
class DataBaseID {

};
// PersonINfo类有若干已实现的函数
// 可用以实现IPerson接口
class PersonInfo {
public:
    explicit PersonInfo(DataBaseID pid);
    virtual ~PersonInfo();
    virtual const char* TheName() const;
    virtual const char* TheBirthDate() const;
    virtual const char* ValueDelimOpen() const;
    virtual const char* ValueDelimClose() const;
};
// CPerson类使用多重继承
class CPerson : public IPerson, private PersonInfo {
public:
    explicit CPerson(DataBaseID pid) : PersonInfo(pid) {

    }
    // 实现必要的IPerson成员函数
    virtual std::string Name() const {

    }
    virtual std::string BirthDate() const {

    }
private:
    // 重新定义继承而来的虚函数
    const char* ValueDelimOpen() const {
        return "";
    }
    const char* ValueDelimClose() const {
        return "";
    }
};

int main() {
    Penguin* p = new Penguin;
    p->Fly();
    
    test1();

}