/*
lambda表达式就是一个表达式，例如
std::find_if(container.begin(), container.end(), [](int val) {
    return 0 < val && val < 10;
});
闭包(enclosure)是lambda创建的运行时对象。依赖捕获模式，闭包持有被捕获数据的
副本或者引用
闭包类(closure class)是从实例化闭包的类。每个lambda都会被编译器生成唯一的闭包类。
lambda中的语句成为其闭包类的成员函数中的可执行指令。
lambda通常被用来创建闭包，该闭包仅用作函数的实参。闭包通常可以拷贝
{
    int x;
    auto c1 = [x](int y) {
        return x* y > 55;
    }; // c1是lambda产生的闭包的副本
    auto c2 = c1;
    auto c3 = c2;
}
c1,c2,c3都是lambda产生的闭包的副本。
*/
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <memory>
/*
item31: 避免使用默认捕获模式
C++11有两种默认的捕获模式：按引用捕获和按值捕获。
但默认按引用捕获模式可能会带来悬空引用的问题，而默认按值捕获模式可能
会诱骗你让你以为能解决悬空引用的问题（实际上并没有），还会让你以为你的闭包是独立的（事实上也不是独立的）

按引用捕获会导致闭包中包含了对某个局部变量或者形参的引用，变量或者形参只在定义lambda的作用域中可用。
如果该lambda创建的闭包生命周期超过了局部变量或者形参的生命周期，那么闭包中的引用将会成为悬空引用。
示例：
过滤函数的一个容器，该函数接受一个int，返回一个bool，该bool的结果表示传入的值是否满足过滤条件

请记住：

1. 默认的按引用捕获可能会导致悬空引用。
2. 默认的按值捕获对于悬空指针很敏感（尤其是this指针），并且它会误导人产生lambda是独立的想法。
*/
using FilterContainer = std::vector<std::function<bool(int)>>;
FilterContainer filters; // 过滤函数

int computeDivisor() {
    int divisor = 5;
    // 计算除数，假设得到结果为5
    return divisor;
}

void addDivisorFilter() {
    auto divisor = computeDivisor();
    filters.emplace_back(
        // 危险！ 对divisor的引用将会悬空
        [&](int value) {
            std::cout << divisor << std::endl;
            return value % divisor == 0;
        }
    );
    // 同样会悬空
    filters.emplace_back(
        [&divisor](int value) {
            std::cout << divisor << std::endl;
            return value % divisor == 0;
        }
    );
    // 一个解决问题的方法，divisor默认按值捕获
    /*
    但是在通常情况下，按值捕获并不能完全解决悬空引用的问题。
    如果按值捕获的是一个指针，将该指针拷贝到lambda对应的闭包里，但这样
    并不能避免lambda外delete这个指针的行为，从而导致你的副本指针变成悬空指针
    */
    filters.emplace_back(
        [=](int value) {
            return value % divisor == 0;
        }
    );
}

/*
如果你知道一个闭包将会被马上使用（例如被传入到一个STL算法中）并且不会被拷贝，
那么在它的lambda被创建的环境中，将不会有持有的引用比局部变量和形参活得长的风险。
在这种情况下，你可能会争论说，没有悬空引用的危险，就不需要避免使用默认的引用捕获模式。

但是这种安全是不确定的。可能会发现其他地方还可以使用它。
*/
template<typename C>
void workWithContainer(const C& container) {
    auto divisor = computeDivisor();

    using ContElemT = typename C::value_type; // 容器内元素的类型
    using std::begin;
    using std::end;

    if(std::all_of(begin(container), end(container), [&](const ContElemT& value) {
        return value % divisor == 0;
    })) {
        std::cout << "container中所有元素都能整除divisor" << std::endl;
    } else {
        std::cout << "container中存在元素不是divisor的倍数" << std::endl;
    }
    // C++14支持在lambda中使用auto声明变量
    // std::all_of(begin(container), end(container), [&](const auto& value)) {
    //     return value % divisor == 0;
    // }
}

/*
捕获只能应用于lambda被创建时所在作用域的non-static局部变量（包括形参）。在
Widget::addFilter的视线中，divisor不是一个局部变量，而是Widget类的一个成员变量。
它不能被捕获，如果默认捕获模式被删除，代码就不能编译了。

这里隐式使用了一个原始指针：this。每一个non-static成员函数都有一个this指针，
每次使用一个类内的数据成员时都会使用这个指针。
例如，在任何Widget成员函数中，编译器会在内部将divisor替换成this->divisor

所以闭包内含有Widget的this指针的拷贝。
*/
class Widget {
public:
    void addFilter() const;
private:
    int divisor;
};
// 定义
void Widget::addFilter() const {
    // filters.emplace_back(
    //     [](int value) {
    //         return value % divisor == 0; // error
    //     }
    // );
    filters.emplace_back(
        [=](int value) {
            return value % divisor == 0;
        }
    );
    // 尝试显式地捕获divisor变量(按引用或按值——这不重要)，也一样会编译失败
    // 因为divisor不是一个局部变量或者形参
    // filters.emplace_back(
    //     [divisor](int value) {
    //         return value % divisor == 0;
    //     }
    // );
}
// 等价于
// void Widget::addFilter() const {
//     auto currentObjectPtr = this;
//     filters.emplace_back(
//         [currentObjectPtr](int value) {
//             return value % currentObjectPtr->divisor == 0;
//         }
//     );
// }

/*
调用doSomeWork时，会创建一个过滤器，其生命周期依赖于由std::make_unique产生的Widget
对象，即一个含有指向Widget的指针——Widget的this指针——的过滤器。
这个过滤器被添加到filters中，但当doSomeWork结束时，Widget会由管理它的std::unique_ptr
来销毁。从这时起，fitler会含有一个存着悬空指针的条目。
*/
void doSomeWork() {
    auto pw = std::make_unique<Widget>();
    pw->addFilter();

}
// 解决方法： 给捕获数据做一个局部副本，捕获这个副本
// void Widget::addFilter() const {
//     auto divisorCopy = divisor;

//     filters.emplace_back(
//         [divisorCopy](int value) {
//             return value % divisorCopy == 0;
//         }
//     );
// }
// 若采用这中方法，默认的按值捕获也是可行的。
// void Widget::addFilter() const {
//     auto divisorCopy = divisor;

//     filters.emplace_back(
//         [=](int value) {
//             return value % divisorCopy == 0;
//         }
//     );
// }
// 在C++14中，一个更好的捕获成员变量的方式是使用通用的lambda捕获：
// void Widget::addFilter() const {
//     filters.emplace_back(
//         // C++14: 拷贝divisor到闭包
//         [divisor = divisor](int value) {
//             return value % divisor == 0;
//         }
//     );
// }

/*
使用默认的按值捕获还有一个缺点：它们预示了相关的闭包是独立的并且不受外部数据变化的影响。
一般来说，这是不对的。lambda可能会依赖局部变量和形参（它们可能被捕获），
还有静态存储生命周期（static storage duration）的对象。
这些对象定义在全局空间或者命名空间，或者在类、函数、文件中声明为static。
这些对象也能在lambda里使用，但它们不能被捕获。
但默认按值捕获可能会因此误导你，让你以为捕获了这些变量。
*/
void addDivisorFilter1() {
    static auto divisor = computeDivisor();

    filters.emplace_back(
        // 什么也没有捕获到！
        [=](int value) {
            // 引用上面的static
            return value % divisor == 0;
        }
    );

    ++divisor;
}

int main() {
    // filters.emplace_back(
    //     [](int value) {
    //         return value % 5 == 0;
    //     }
    // );
    // 然而可能需要的是能够在运行期计算除数，即不能将5硬编码到lambda中。
    {
        addDivisorFilter();
        int x = filters[0](11);
        std::cout << x << std::endl;
        /*
        lambda对局部变量divisor进行了引用，但是该变量的生命周期在addDivisorFilter返回后结束，
        刚好就在语句filters.emplace_back返回之后。
        因此添加到filters的函数添加完，该函数就死亡了。
        使用这个过滤器会导致未定义行为
        */
        // 同样的问题也会出现在divisor的显式按引用捕获 
        x = filters[1](10);
        std::cout << x << std::endl;
    }
    using ContElemT = std::vector<int>::value_type;
}