/*
item23: 理解std::move和std::forward

请记住：
1. std::move执行到右值的无条件的转换，但就自身而言，它不移动任何东西。
2. std::forward只有当它的参数被绑定到一个右值时，才将参数转换为右值。
3. std::move和std::forward在运行期什么也不做。

移动语义：使编译器有可能用廉价的移动操作来代替昂贵的拷贝操作。
移动语义也允许创建只可移动的类型，例如std::unique_ptr, std::future, std::thread

完美转发：使接收任意数量实参的函数模板成为可能，它可以将实参转发到其他的函数，
使目标函数接收到的实参与被传递给转发函数的实参保持一致。

右值引用是连接这两个截然不同的概念的胶合剂。它是使移动语义和完美转发变成可能的基础语言机制。

std::move并不移动任何东西，完美转发也并不完美。移动操作并不永远比复制操作更廉价；即便如此，
它也并不总是像你期望的那么廉价。而且，它也并不总是被调用，即使当移动操作可用的时候。
构造type&&也并非总是代表一个右值引用。

非常重要的一点： 形参永远是左值，即便它的类型是一个右值引用
例如
void f(Widget&& w);
形参w是一个左值，即便它的类型是一个rvalue-reference-to-Widget

std::move不移动（move）任何东西，std::forward也不转发（forward）任何东西
在运行时，它们不做任何事情。它们不产生任何可执行代码，一字节都没有。
std::move和std::forward仅仅执行转换（cast）的函数（事实上是函数模板）
std::move无条件将它的实参转换为右值，而std::forward只有在特定情况满足时才进行转换。

std::move的使用代表着无条件向右值转换，而使用std::forward只对绑定了右值的引用进行右值转换。
这是两种不同的动作。前者是典型的为了移动操作，而后者只是传递（亦或转发）一个对象到另一个函数，
保留它原有的左值属性或右值属性。

*/

#include <iostream>
#include <type_traits>
#include <chrono>

// C++11的std::move的示例实现，并不完全满足标准细则，但是它非常接近了
/*
std::move接受一个对象的引用（准确的说，一个通用引用），返回一个指向同对象的引用
函数返回类型的&&部分表明std::move函数返回的是一个右值引用。
如果类型T是一个左值引用，那么T&&将会成为一个左值引用。
为了避免如此，type trait std::remove_reference应用到类型T上，因此确保了&&被正确的
应用到一个不是引用的类型上，这保证了std::move返回的真的是右值引用，所以函数返回的右值
引用是右值。
因此，std::move将它的实参转换为一个右值，这就是它的全部作用。
*/
template<typename T>
typename std::remove_reference<T>::type&& move1(T&& param) {
    using ReturnType = typename std::remove_reference<T>::type&&;
    return static_cast<ReturnType>(param);
}
// 在C++14中实现更加简单
template<typename T>
decltype(auto) move2(T&& param) {
    using ReturnType = std::remove_reference_t<T>&&;
    return static_cast<ReturnType>(param);
}

// 右值只不过经常是移动操作的候选者
/*
下面例子可总结2点。
1. 不要在希望能移动对象时，声明他们为const。对const对象的移动请求会被悄无声息的转化为拷贝操作。
2. std::move不仅不会移动任何东西，而且它也不保证它执行转换的对象可以被移动。
关于std::move，能确保的唯一一件事是将它应用到一个对象上，能够得到一个右值
*/
class Annotation {
public:
    // 当复制数据时不会修改text，因此声明为const
    // 复制text到value，为了避免一次复制操作的代价使用std::move应用到text
    // 实际上，text并不是被移动到了value，而是被拷贝
    // text虽然通过std::move被转换为右值，但是text被声明为const std::string
    // 所以转换前，text是一个左值的const std::string, 而转换的结果是一个右值的const std::string，
    // 所以const属性被一直保留。
    /*
    当编译器决定哪一个std::string构造函数被调用时，考虑它的作用，将有两种可能性
    class String {
    public:
        string(const string& rhs); // 拷贝构造函数
        string(string&& rhs); // 移动构造函数
    };
    std::move(text)的结果是一个const std::string右值，这个右值不能传递给std::string的移动构造函数
    因为移动构造函数只接受一个指向non-const string的右值引用。然而，该右值却可以传递给string
    的拷贝构造函数.
    所以string在成员初始化的过程中调用了拷贝构造函数，即使text已经被转换成了右值。这是为了确保
    维持const属性的正确性
    */
    explicit Annotation(const std::string text) : value(std::move(text)) {
        
    }
private:
    std::string value;
};

/*
关于std::forward的故事与std::move是相似的，但是与std::move总是无条件的将它的实参为右值不同，
std::forward只有在满足一定条件的情况下才执行转换。
std::forward是有条件的转换。要明白什么时候它执行转换，什么时候不，
想想std::forward的典型用法。最常见的情景是一个模板函数，接收一个通用引用形参，并将它传递给另外的函数：
*/
class Widget {

};
// 处理左值
void process(const Widget& lvalArg) {
    std::cout << "process(const Widget&)" << std::endl;
}
// 处理右值
void process(Widget&& rvalArg) {
    std::cout << "process(Widget&&)" << std::endl;
}
// 用以转发param到proces的模板
/*
param是左值。每次在函数logAndProcess内部对函数process的调用，
都会因此调用函数process的左值重载版本。为防如此，需要一种机制：
当且仅当传递给函数logAndProcess的用以初始化param的实参是一个右值时，param
会被转换成一个右值。这就是std::forward做的事情。
所以std::forward是一个有条件的转换：它的实参用右值初始化时，转换为一个右值

但是std::forward怎么知道它的实参是否是被一个右值初始化的？
简短的说，该信息隐藏在函数logAndProcess的模板参数T中。
该参数被传递给了std::forward，它解开了含在其中的信息。
该机制工作细节可以查询item28
*/
template<typename T>
void logAndProcess(T&& param) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);

    std::cout << buffer << std::endl;
    process(std::forward<T>(param));
    
}

int main() {
    const int&& x = 10;
    const int& y = x;

    Widget w;
    logAndProcess(w); // 用左值调用
    logAndProcess(std::move(w)); // 用右值调用
}