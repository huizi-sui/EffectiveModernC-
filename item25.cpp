/*
item25: 对右值引用使用std::move，对通用引用使用std::forward

请记住：

1. 最后一次使用时，在右值引用上使用std::move，在通用引用上使用std::forward。
2. 对按值返回的函数要返回的右值引用和通用引用，执行相同的操作。
3. 如果局部对象可以被返回值优化消除，就绝不使用std::move或者std::forward。
*/

#include <iostream>
#include <memory>

struct SomeDataStructure
{
    /* data */
};


// 右值引用仅绑定可以移动的对象
// 总结：
// 当把右值引用转发给其他函数时，右值引用应当被无条件转换为右值（通过std::move)，因为它们总是绑定到右值
// 当转发通用引用时，通用引用应当有条件地转换为右值（通过std::forward），因为它们只是有时绑定到右值
class Widget {
public:
    Widget() {

    }
    Widget(Widget&& rhs) : name(std::move(rhs.name)), p(std::move(rhs.p)) { // rhs定义上引用一个有资格移动的对象

    }
    // 另一方面，通用引用可能绑定到有资格移动的对象上。
    // 通用引用使用右值初始化时，才将其强制转换为右值
    template<typename T>
    void setName(T&& newName) { // newName是通用引用
        name = std::forward<T>(newName);
    }
private:
    std::string name;
    std::shared_ptr<SomeDataStructure> p;
};

// 通用引用上使用std::move很糟糕，可能会意外改变左值
class Widget1 {
public:
    template<typename T>
    void setName(T&& newName) { // 通用引用可以编译，但是代码太差
        name = std::move(name);
    }
private:
    std::string name;
};

// 工厂函数
std::string getWidgetName() {
    std::string s;
    s.append("12345");
    return s;
}

/*
也可以工作，但是有缺点
1. 编写和维护的代码更多（两个函数而不是单个模板）
2. 效率下降。 例如w.setName("Adela Novak")
对左值和右值的重载函数更重要的问题是设计的可扩展性差。
有一个形参，需要两种重载实现
n个参数，就要实现2^n种。
有的函数——实际上是函数模板——接受无限制个数的参数，每个参数都可以是左值或者右值
典型是std::make_shared，还有对于C++14的std::make_unique，它们肯定使用
std::forward传递通用引用形参给其他函数
*/
class Widget2 {
public:
    // 用const左值设置
    void setName(const std::string& newName) {
        name = newName;
    }
    // 用右值设置
    void setName(std::string&& newName) {
        name = std::move(newName);
    }
private:
    std::string name;
};

template<class T, class... Args> // C++11标准
std::shared_ptr<T> make_shared(Args&&... args) {

}
template<class T, class... Args> // C++14标准
std::unique_ptr<T> make_unique(Args&&... args) {

}

// 在按值返回的函数中，返回值绑定到右值引用或者通用引用上，需要对返回的引用
// 使用std::move或者std::forward. 考虑矩阵加法operator+，左侧矩阵为右值（可以被用来保存求值知乎的和）
class Matrix {
public:
    Matrix operator+=(const Matrix& rhs) {
        

        return *this;
    }
    Matrix operator-=(const Matrix& rhs) {
        

        return *this;
    }
};
// 通过在return语句中将lhs转化为右值（通过std::move)，lhs可以移动到返回值的内存位置
// 如果忽略了std::move的调用，lhs是个左值的事实，会强制编译器拷贝它到返回值的内存空间
// 假设Matrix支持移动操作，并且比拷贝操作效率更高，在return语句中使用std::move的代码效率更高
// 如果Matrix不支持移动操作，将其转换为右值不会变差，因为右值可以直接被Matrix的拷贝构造函数拷贝。
// 如果Matrix随后支持了移动操作，operator+将在下一次编译时受益。
// 按值返回
Matrix operator+(Matrix&& lhs, const Matrix& rhs) {
    lhs += rhs;
    return std::move(lhs); //移动lhs到返回值
}
Matrix operator-(Matrix&& lhs, const Matrix& rhs) {
    lhs -= rhs;
    return lhs; // 拷贝lhs到返回值
}
// 使用通用引用和std::forward的情况类似
class Fraction {
public:
    void reduce() {

    }
};
// 按值返回，通用引用的形参
template<typename T>
Fraction reduceAndCopy(T&& frac) {
    frac.reduce();
    return std::forward<T>(frac); // 移动右值，或拷贝左值到返回值
    // 若std::forward被忽略，则frac就会被无条件复制到reduceAndCopy的返回值内存空间
}

Widget makeWidget() {
    Widget w;
    // ...
    return w; // “拷贝”w到返回值
}
// 优化代码,把“拷贝”变为移动
// Widget makeWidget() {
//     Widget w;
//     // ...
//     return std::move(w); // ！不应该这么做
// }
// 通过在分配给函数返回值的内存中构造w来实现，这就是所谓的返回值优化(return value optimization, RVO)
// 这已经在C++标准中实现了

/*
编译器可能会在按值返回的函数中消除对局部对象的拷贝或者移动
满足
1. 局部对象与函数返回值的类型相同
2. 局部对象就是要返回的东西，还有作为return语句的一部分而创建的临时对象
函数形参不满足要求。
上面移动版本的makeWidget将w的内容移动到makeWidget的返回值位置。
不适用RVO消除这种移动，因为不满足条件2。
return std::move(w);返回的不是局部对象，而是w的引用
*/
/*
C++标准关于RVO的部分表明，如果满足RVO的条件，但是编译器选择不执行拷贝消除，
则返回的对象必须被视为右值。
实际上，标准要求当RVO被允许时，或者实行拷贝消除，或者将std::move隐式应用于
返回的局部对象。
*/

int main() {
    {
        Widget w;
        std::string n = getWidgetName(); // 局部变量
        w.setName(n);
        std::cout << n << std::endl;
    }
    {
        Widget1 w1;
        auto n = getWidgetName(); // 局部变量
        w1.setName(n); // 把n移动进w
        std::cout << n << std::endl; // 现在n的值未知???,打印输出没有改变
    }
    {
        int x = 10;
        int y = std::move(x);
        std::cout << x << std::endl;
    }
    {
        /*
        使用通用引用的版本的setName，字面字符串“Adela Novak”可以被传递给setName，
        再传给w内部std::string的赋值运算符。
        w的name的数据成员通过字面字符串直接赋值，没有临时std::string对象被创建。
        */
        /*
        但是，setName重载版本，会有一个临时std::string对象被创建，
        setName形参绑定到这个对象，然后这个临时std::string移动到w的数据成员中。
        一次setName的调用会包括std::string构造函数调用（创建中间对象），
        std::string赋值运算符调用（移动newName到w.name），
        std::string析构函数调用（析构中间对象）。
        */
        Widget2 w;
        w.setName("Adela Novak");
    }
}