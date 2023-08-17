/*
item15: 尽可能使用constexpr

请记住：
1. constexpr对象是const，它被在编译期可知的值初始化
2. 当传递编译期可知的值时，constexpr函数可以产出编译期可知的结果
3. constexpr对象和函数可以使用的范围比non-constexpr对象和函数要大
4，constexpr是对象和函数接口的一部分

当用于对象上面，它本质上就是const的加强形式，但是当它用于函数上，意思就大不相同了。

从概念上来说，comstexpr表明一个值不仅仅是常量，还是编译期可知的。
这个表述不全面，当constexpr被用于函数时，事情有一些细微差别
不能假设constexpr函数的结果是const，也不能保证它们的返回值在编译期可知
这些是特性。关于constexpr函数返回的结果不需要是const，也不需要编译期可知这一点是良好的行为。

先从constexpr对象开始说起。这些对象，实际上，和const一样，它们是编译期可知的
编译期可知的值“享有特权”，它们可能被存放到只读存储空间中。

涉及到constexpr函数时，constexpr对象使用情况就更有趣了。如果实参是编译器常量，这些函数
将产出编译器常量；如果实参是运行时才能知道的值，将产出运行时值。
1. constexpr函数可以用于需求编译期常量的上下文。如果你传给constexpr函数的实参在编译期可知，
那么结果将在编译期计算。如果实参的值在编译期不知道，则代码就会被拒绝
2. 当一个constexpr函数被一个或者多个编译期不可知值调用时，它就像普通函数一样，运行时计算它的结果。
这意味着你不需要两个函数，一个用于编译期计算，一个用于运行时计算。constexpr全做了。

因为constexpr函数必须能在编译期调用的时候返回编译期结果，就必须对它的实现施加一些限制。
在C++11中， constexpr函数的代码不超过1行语句：一个return。
两个技巧可以扩展constexpr函数的表达能力
1. 使用三元运算符代替if-else语句
2，使用递归代替循环
在C++14实现很宽松，constexpr函数限制为只能获取和返回字面值类型，这基本上意味着那些有了值
的类型能在编译期决定。
在C++11中，除了void外的所有内置类型，以及一些用户定义类型都可以是字面值类型，因为构造函数
和其他成员函数可能是constexpr
在C++11中，有两个限制使得Point的成员函数setX和setY不能声明为constexpr。
1. 它们修改它们操作uix的状态，并且在C++11中，constexpr成员函数是隐式的const
2. 它们有void返回类型，void类型不是C++11中的字面值类型
这两个限制在C++14中放开了，所以C++14中的Point的setter也能声明为constexpr
*/
#include <iostream>
#include <array>

constexpr int pow(int base, int exp) noexcept {
    return (exp == 0 ? 1 : base * pow(base, exp - 1));
}

class Point {
public:
    // 构造函数可以被声明为constexpr，因为如果传入的参数在编译期可知，
    // Point的数据成员也能在编译期可知。因此这样初始化的Point就能是constexpr
    constexpr Point(double xVal = 0, double yVal = 0) noexcept : x(xVal), y(yVal) {

    }
    // x和y的getter函数也能是constexpr，因为如果对一个编译期已知的Point对象调用getter
    // 数据成员x和y的值也能在编译期知道
    constexpr double xValue() const noexcept {
        return x;
    }
    constexpr double yValue() const noexcept {
       return y; 
    }
    void setX(double newX) noexcept {
        x = newX;
    }
    void setY(double newY) noexcept {
        y = newY;
    }
    // C++14
    // constexpr void setX(double newX) noexcept {
    //     x = newX;
    // }
    // constexpr void setY(double newY) noexcept {
    //     y = newY;
    // }
private:
    double x, y;
};

constexpr Point midpoint(const Point& p1, const Point& p2) noexcept {
    // 调用constexpr函数
    return { (p1.xValue() + p2.xValue()) / 2,
             (p1.yValue() + p2.yValue() / 2) };
}

// C++14
// 返回p相对于原点的对象
// constexpr Point reflection(const Point& p) noexcept {
//     Point result; // 创建non-const Point
//     result.setX(-p.xValue());
//     result.setY(-p.yValue());
//     return result; // 返回它的副本
// }

int main() {
    // 简而言之，所有constexpr对象都是const，但不是所有const对象都是constexpr。
    // 如果想编译器保证一个变量有一个值，这个值可以放到那些需要编译器常量（compile-time constants）的
    // 上下文的地方，需要的工具是constexpr而不是const
    {
        int sz; // non-constexpr变量
        // constexpr auto arraySize1 = sz; // error, sz的值在编译器不可知
        // std::array<int, sz> data1; // error, 一样的问题
        constexpr auto arraySize2 = 10;
        std::array<int, arraySize2> data2;
        // 注意，const不提供constexpr所能保证之事，
        // 因此const对象不需要在编译器初始化它的值
        {
            int sz;
            const auto arraySize = sz; // 没问题，arraySize是sz的const复制
            // std::array<int, arraySize> data; // error, arraySize值在编译器不可知
        }
        {
            constexpr auto numConds = 5;
            // 编译期调用函数
            std::array<int, pow(3, numConds)> results; // 结果有3^numConds个元素
            // 运行时调用函数
            int base = 2;
            int exp =3;
            auto baseToExp = pow(base, exp);
        }
    }
    {
        constexpr Point p1(9.4, 27.7); // 没问题，constexpr构造函数会在编译期“运行”
        constexpr Point p2(28.8, 5.3); // 也没问题
        constexpr auto mid = midpoint(p1, p2); // 使用constexpr函数的结果初始化constexpr对象
        // constexpr auto reflectedMid = reflection(mid); // C++14, reflectedMid的值在编译期可知
    }
}