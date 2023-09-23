#include <iostream>
#include <array>
/*
item41: 了解隐式接口和编译器多态
类与模板都支持接口和多态。对于类而言接口是显式的，以函数签名为中心，
多态则是通过虚函数发生于运行期。
对于模板参数而言，接口是隐式的，奠基于有效表达式，多态则是通过模板
具现化和函数重载解析(function overloading resolution)发生于编译期。
*/
// template<typename T>
// void DoProcessing(T& w) {
//     if(w.size() > 10 && w != someNastyhWidget) {

//     }
// }
/*
上面代码中，T类型的隐式接口要求：
1. 提供一个名为size的成员函数，该函数的返回值可于int执行operator >,
或经过隐式转换后可以执行operator>
2. 必须支持一个operator!=函数，接受T类型和someNastyWidget的类型，
或其隐式转换后得到的类型。
(这里没有考虑operator&&被重载的可能性)

加诸于模板参数身上的隐式接口，就像加诸于类对象身上的显式接口“一样真实”，
两者都在编译期完成检查，无法在模板中使用“不支持模板所要求之隐式接口”
的对象(代码无法通过编译)。
*/

/*
item42：了解typename的双重含义
在模板声明式中，使用class和typename关键字没有什么区别，但在模板内部，
typename拥有更多的一重含义。

概念：模板内出现的名称如果依赖于某个模板参数，称之为从属名称(dependent names)；
如果从属名称在类内呈嵌套状，称之为嵌套从属名称(nested dependent names),
如果一个名称并不依赖任何模板参数的名称，称之为非从属名称(non-dependent names)
*/
template<typename C>
void Print2nd(const C& container) {
    if(container.size() >= 2) {
        // 编译报错
        /*
        C::const_iterator是一个指向某个类型的嵌套从属名称，而嵌套从属名称可能会
        导致解析困难，因为在编译器知道C是什么之前，没有任何办法知道C::const_iterator
        是否是一个类型，这就导致出现了歧义状态，而C++默认假设嵌套从属名称不是类型名称。

        显式指明嵌套从属类型名称的方法就使讲typename关键字作为其前缀词。
        */
        // C::const_iterator iter(container.begin());
        typename C::const_iterator iter(container.begin());
        ++iter;
        int value = *iter;
        std::cout << value;
    }
}
// 同样，若嵌套从属名称出现在模板函数声明部分，也需要显式地指明是否为类型名称
template<typename C>
void Print2nd1(const C& container, const typename C::iterator iter);
// 这一规则地例外是，typename不可以出现在基类列表内的嵌套从属类型名称之前，
// 也不可以在成员初始化列表中作为基类的修饰符
template<typename T>
class Base {
public:
    class Nested {
    public:
        Nested(int x) {

        }
    };
};
template<typename T>
class Derived : public Base<T>::Nested { // 基类列表中不允许使用typename
public:
    // 成员初始化列表中不允许使用typename
    explicit Derived(int x) : Base<T>::Nested(x) {
        typename Base<T>::Nested temp;
    }
};
// 在类型名称过于复杂时，可以使用using或typedef来进行简化
// using value_type = typename std::iterator_traits<IterT>::value_type;

/*
item43: 学习处理模板化基类内的名称
在模板编程中，模板类的继承并不像普通类那么自然
*/
class MsgInfo {

};
template<typename Company>
class MsgSender {
public:
    void SendClear(const MsgInfo& info) {

    }
};
template<typename Company>
class LoggingMsgSender : public MsgSender<Company> {
public:
    // 2. 使用using声明式
    using MsgSender<Company>::SendClear;
    void SendClearMsg(const MsgInfo& info) {
        // SendClear(info); // 调用基类函数，无法通过编译
        /*
        由于直到模板类被真正实例化之前，编译器并不知道MsgSender<Company>具体长什么样，
        有可能它是一个全特化的版本，而在这个版本中不存在SendClear函数。
        由于C++的设计策略是宁愿较早进行诊断，所以编译器会拒绝承认在基类中存在
        一个SendClear函数。
        为了解决这个问题，需要零C++“进入模板基类观察”的行为生效，有3种办法。
        */
       // 1. 在基类函数调用动作之前加上this->,它指明了SendClear函数是从基类继承而来的
        this->SendClear(info);
       // 2. 使用using声明式
        SendClear(info);
       // 3. 明确指出被调用的函数位于基类内
       // 最不能让人满意，如果被调用的是虚函数，明确资格修饰会使
       // “虚函数绑定行为”失效。
        MsgSender<Company>::SendClear(info);
    }
};
/*
item44：将与参数无关的代码抽离模板
模板可以节省时间和避免代码重复，编译器会为填入的每个不同模板参数具现化出一份
对应的代码，但除此之外，可能会造成代码膨胀，生成浮夸的二进制目标码。
基于共性和变性分析(commonality and variability analysis)的方法，
需要分析模板中重复使用的部分，将其抽离出模板，以减轻模板具现化带来的代码量。
1. 因非类型模板参数造成的代码膨胀，往往可以消除，做法是以函数参数或类成员变量替换模板参数
2. 因类型模板参数而造成的代码膨胀，往往可以降低，做法是让带有完全相同二进制表述的具现类型
共享实现代码。
*/
template<typename T, std::size_t n>
class SquareMatrix {
public:
    void Invert();
private:
    std::array<T, n*n> data;
};
/*
SquareMatrix<double, 5> sm1;
SquareMatrix<double, 10> sm2;
sm1.invert();
sm2.invert();
会具现出两个invert并且基本完全相同
*/
// 修改为：
template<typename T>
class SquareMatrixBase {
protected:
    SquareMatrixBase(T* p) : dataPointer(p) {

    }
    void Invert(std::size_t matrixSize);
private:
    T* dataPointer;
};
template<typename T, std::size_t n>
class SquareMatrix1 : private SquareMatrixBase<T> {
public:
    SquareMatrix1() : SquareMatrixBase<T>(&data) {

    }
    void Invert() {
        Invert(n);
    }
private:
    using SquareMatrixBase<T>::Invert; // 避免遮掩基类函数，item33
    std::array<T, n * n> data;
};
// Invert并不是我们唯一要使用的矩阵操作函数，而且每次都往基类传递矩阵尺寸显得太过繁琐
// 可以考虑将数据放在派生类中，在基类中存储指针和矩阵尺寸。
template<typename T>
class SquareMatrixBase1 {
protected:
    SquareMatrixBase1(std::size_t n, T* pMem) : size(n), pData(pMem) {

    }
    void SetDataPth(T* ptr) {
        pData = ptr;
    }
    void invert() {
        std::cout << "SquareMatrixBase1::invert" << std::endl;
    }
private:
    std::size_t size;
    T* pData;
};
template<typename T, std::size_t n>
class SquareMatrix2 : private SquareMatrixBase1<T> {
public:
    SquareMatrix2() : SquareMatrixBase1<T>(n, data.data()) {

    }
    void invert() {
        std::cout << "SquareMatrix2::invert\n";
        SquareMatrixBase1<T>::invert();
    }
private:
    std::array<T, n * n> data;
};

/*
item45: 运用成员函数模板接受所有兼容类型
C++视模板类的不同具现体为完全不同的类型，但在泛型编程中，
可能需要一个模板类的不同具现体能够相互类型转换。

考虑设计一个智能指针类，而智能指针需要支持不同类型指针之间的隐式转换(如果可以的话)，
以及普通指针到智能指针的显式转换。需要的是模板拷贝构造函数
*/
template<typename T>
class SmartPtr {
public:
    template<typename U>
    SmartPtr(const SmartPtr<U>& other) : heldPtr(other.get()) {

    }
    template<typename U>
    explicit SmartPtr(U* p) : heldPtr(p) {

    }

    T* get() const {
        return heldPtr;
    }
private:
    T* heldPtr;
};
// 使用get获取原始指针，并将在原始指针之间进行类型转换本身提供了一种保障，
// 如果原始指针之间不能隐式转换，那么其对应的智能指针之间的隐式转换会造成编译错误。
// 模板构造函数并不会阻止编译器暗自生成默认的构造函数，若想要控制拷贝构造的方方面面，
// 必须同时声明泛化拷贝构造函数和普通拷贝构造函数，相同规则也适用于赋值运算符
template<typename T>
class shared_ptr {
public:
    // 拷贝构造函数
    shared_ptr(const shared_ptr& r);
    // 泛化拷贝构造函数
    template<typename Y>
    shared_ptr(const shared_ptr<Y>& r);
    // 拷贝赋值运算符
    shared_ptr& operator=(const shared_ptr& r);
    // 泛化拷贝赋值运算符
    template<typename Y>
    shared_ptr& operator=(const shared_ptr<Y>& r);
};

/*
item46: 需要类型转换时请为模板定义非成员函数
与item24一脉相承
*/
template<typename T>
class Rational;

template<typename T>
const Rational<T> DoMultiply(const Rational<T>& lhs, const Rational<T>& rhs) {
    return Rational<T>(lhs.Numberator() * rhs.Numberator(), lhs.Denominator() * rhs.Denominator());
}

template<typename T>
class Rational {
public:
    Rational(const T& numberator = 0, const T& denominator = 1) : numberator_(numberator),
            denominator_(denominator) {

    }
    const T& Numberator() const {
        return numberator_;
    }
    const T& Denominator() const {
        return denominator_;
    }

    friend const Rational<T> operator*(const Rational<T>& lhs, const Rational<T>& rhs) {
        return DoMultiply(lhs, rhs);
    }
    // 在模板类内，模板名称可被用来作为“模板及其参数”的简略表达形式
    // 因此等价于下面写法
    // friend const Rational operator*(const Rational& lhs, const Rational& rhs);
    // 为了使程序能正常链接，需要为其提供对应的定义式，最简单有效的方法是直接合并至声明式处
    // friend const Rational operator*(const Rational& lhs, const Rational& rhs) {
    //     return Rational(lhs.Numberator() * rhs.Numberator(), lhs.Denominator() * rhs.Denominator());
    // }
    // 但是定义在类内的函数都会暗自称为内联函数，为了降低内联的冲击，可以使operator*调用类外的辅助模板函数
private:
    T numberator_;
    T denominator_;
};
template<typename T>
const Rational<T> operator*(const Rational<T>& lhs, const Rational<T>& rhs) {
    return Rational<T>(lhs.Numberator() * rhs.Numberator(), lhs.Denominator() * lhs.Denominator());
}
void test() {
    Rational<int> oneHalf(1, 2);
    // Rational<int> result = oneHalf * 2; // 无法通过编译
    /*
    上面失败：模板实参在推导过程中，从不将隐式类型转换纳入考虑。
    虽然以oneHalf推导出Rational<int>类型是可行的，但是试图将int类型隐式转换
    为Ration<T>是绝对会失败的
    
    由于模板类不依赖模板实参推导，所以编译器总能够在Rational<T>具现化时得知T，
    因此可以使用友元声明式在模板类指定特定函数
    */
    /*
    当对象oneHalf被声明为一个Rational<int>时，Rational<int>类于是被具现化出来，
    而作为过程的一部分，友元函数operator*也就被自动声明出来，其为一个普通函数而非
    模板函数，因此在接受参数时可以正常执行隐式转换
    */
    Rational<int> reslut = oneHalf * 2;
}

/*
item47: 请使用traits classes表现类型信息
traits classes可以使我们在编译期就能获取某些类型信息。
traits并不是C++关键字或一个预先定义好的构件：他们是一种技术，
也是C++程序员所共同遵守的协议，并要求对用户自定义类型和内置类型表现得一样好。

设计并实现一个traits class的步骤：
1. 确认若干希望将来可取得的类型相关信息。
2. 为该类型选择一个名称。
3. 提供一个模板和一组特化版本，内含你希望支持的类型相关信息。
以迭代器为例，标准库中拥有多种不同的迭代器种类，它们各自拥有不同的功用和限制：
1. input_iterator_tag：单向输入迭代器，只能向前移动，一次一步，客户只可读取它所指的东西。
2. output_iterator_tag：单项输出迭代器，只能向前移动，一次一步，客户只可写入它所指的东西。
3. forward_iterator_tag: 单项访问迭代器，只能向前移动，一次一步，读写均可。
4. bidirectional_iterator_tag: 双向访问迭代器，去除了只能向前移动的限制。
5. random_access_iterator_tag: 随机访问迭代器，没有一次一步的限制，允许随意移动，可以执行“迭代器算术”。

标准库为这些迭代器种类提供的卷标结构体(tag struct) 的继承关系如下：
*/
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : input_iterator_tag {};
struct bidirectional_iterator_tag : forward_iterator_tag {};
struct random_access_iterator_tag : bidirectional_iterator_tag {};
// 将iteartor_category作为迭代器种类的名称，嵌入容器的迭代器中，并且确认使用适当的
// 卷标结构体
template<typename T>
class deque {
public:
    class iterator {
    public:
        using iterator_category = random_access_iterator_tag;
    };
};
template<typename T>
class list {
public:
    class iterator {
    public:
        using iterator_category = bidirectional_iterator_tag;
    };
};
// 为了做到类型的traits信息可以在类型自身之外获得，标准技术是把它放进
// 一个模板极其一个或多个特化版本中。这样的模板在标准库有若干个，
// 其中针对迭代器的是iterator_traits
template<class IterT>
struct iterator_traits {
    using iterator_category = typename IterT::iterator_category;
};
// 为了支持指针迭代器，iterator_traits特别针对指针类型提供一个偏特化版本
// 而指针的类型和随机访问迭代器类似
template<class IterT>
struct iterator_traits<IterT*> {
    using iterator_category = random_access_iterator_tag;
};
// 当我们需要为不同的迭代器种类应用不同的代码时，traits classes就派上用场了
template<typename IterT, typename DisT>
void advance1(IterT& iter, DisT d) {
    if(typeid(typename std::iterator_traits<IterT>::iterator_category()) == 
        typeid(std::random_access_iterator_tag)) {
        std::cout << "advance1 random access iterator" << std::endl;
    }
    std::cout << "advance1 xxx" << std::endl;
}
// 但这些代码实际上是错误的，我们希望类型的判断能在编译期完成。
// iterator_category是在编译期决定的，然而if却是在运行期运行的。
// 无法达成我们的目标。

// 在C++17之前，解决这个问题的主流做法是利用函数重载(原书中介绍的做法)
template<typename IterT, typename DisT>
void doAdvance(IterT& iter, DisT d, std::random_access_iterator_tag) {
    std::cout << "random access iterator" << std::endl;
}
template<typename IterT, typename DisT>
void doAdvance(IterT& iter, DisT d, std::bidirectional_iterator_tag) {
    std::cout << "bidirectinal iterator" << std::endl;
}
template<typename IterT, typename DisT>
void doAdvance(IterT& iter, DisT d, std::input_iterator_tag) {
    if(d < 0) {
        throw std::out_of_range("Negative distance"); // 单向迭代器不允许负距离
    }
    std::cout << "input iterator\n";
}
template<typename IterT, typename DisT>
void advance(IterT& iter, DisT d) {
    std::cout << "advance\n";
    doAdvance(iter, d, typename std::iterator_traits<IterT>::iterator_category());
}
// 在C++17之后，有了更简单有效的做法，使用if constexpr
// template<typename IterT, typename DisT>
// void Advance(IterT& iter, DisT d) {
//     if constexpr(typeid(typename std::iterator_traits<IterT>::iterator_category())
//             == typeid(std::random_access_iterator_tag)) {
//         std::cout << "random access iterator" << std::endl;
//     } else {
//         std::cout << "not a random access iterator" << std::endl;
//     }
// }

/*
item48: 认识模板元编程
模板元编程(Template metaprogramming，TMP)是编写基于模板的C++程序
并执行于编译期的过程，并不是刻意被设计出来的，而是当初C++引入模板带来的
副产品，事实证明模板元编程具有强大的作用，并且成为C++标准的一部分。
在item47中编写traits classes时，就是在进行模板元编程。

由于模板元程序执行于C++编译期，因此可以将一些工作从运行期转移至编译期，
这可以帮助我们在编译期发现一些原本要在运行期才能察觉的错误，以及得到较小的
可执行文件、较短的运行期、较少的内存需求。当然，副作用是会使编译时间变长。

模板元编程已被证明是“图灵完备”的，并且以“函数式语言”的形式发挥作用，
因此在模板元编程中没有真正意义上的循环，所有循环效果只能藉由递归实现，
而递归在模板元编程中是由 “递归模板具现化（recursive template instantiation）” 实现的。

常用于引入模板元编程的例子是在编译期计算阶乘
*/
template<unsigned n>
struct Factorial {
    enum {
        value = n * Factorial<n-1>::value
    };
};
template<> // 处理特殊情况
struct Factorial<0> {
    enum {
        value = 1
    };
};
/*
模板元编程很酷，但对其进行调试可能是灾难性的，因此在实际应用中并不常见。
可能在以下几种情形下见到它的出场：
1. 确保度量单位正确
2. 优化矩阵计算
3. 可以生成客户定制之设计模式实现品
*/

int main() {
    SquareMatrix2<int, 2> sm;
    sm.invert();

    int arr[] = {1, 2, 3, 4, 5};
    int* ptr = arr;
    advance(ptr, 3);
    advance1(ptr, 2);

    std::cout << Factorial<5>::value << std::endl;
}