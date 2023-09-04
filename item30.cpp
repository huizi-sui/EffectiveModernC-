/*
item30: 熟悉完美转发失败的情况

1. 花括号初始化器
2. 0或者NULL作为空指针。 item8说明当试图传递0或者NULL作为空指针给模板时，类型推导会出错，
会把传来的实参推导为一个整型类型（典型情况是int）而不是指针类型。结果就是不管是0还是NULL都不能作为
空指针被完美转发。解决方案是使用nullptr
3. 仅有声明的整型static const数据成员
4. 重载函数的名称和模板名称
5. 位域。 函数实参使用位域这种类型

在大多数情况下，完美转发工作的很好。你基本不用考虑其他问题。
但是当其不工作时——当看起来合理的代码无法编译，或者更糟的是，
虽能编译但无法按照预期运行时——了解完美转发的缺陷就很重要了。
同样重要的是如何解决它们。在大多数情况下，都很简单。

请记住：

1. 当模板类型推导失败或者推导出错误类型，完美转发会失败。
2. 导致完美转发失败的实参种类有花括号初始化，
作为空指针的0或者NULL，仅有声明的整型static const数据成员，模板和重载函数的名字，位域。
*/

#include <iostream>
#include <vector>

void f(const std::vector<int>& v) {
    std::cout << "f(const std::vector<int>&)" << std::endl;
}

void f(std::size_t val) {
    std::cout << "f(std::size_t)" << std::endl;
}

void f(int (*pf)(int)) {
    std::cout << "f1(int(*pf)(int))" << std::endl;
}
// 上面等价于
// void f1(int pf(int)) {
//     std::cout << "f1(int pf(int))" << std::endl;
// }

template<typename... Ts>
void fwd(Ts&&... params) {
    f(std::forward<Ts>(params)...);
}

// 通常无需在类中定义整型static const数据成员，声明即可。
// 这是因为编译器会对此类成员实行常量传播，因此消除了保留内存的需要。
class Widget {
public:
    static const std::size_t MinVals = 28; // MinVals的声明
    static int x;
};
// 没有MinVals的定义
int Widget::x = 20;
const std::size_t Widget::MinVals; // 添加定义

int processVal(int value) {
    std::cout << "processVal(int)" << std::endl;
    return 0;
}

int processVal(int value, int priority) {
    std::cout << "processVal(int, int)" << std::endl;
    return 0;
}

template<typename T>
T workOnVal(T param) {
    return param;
}

// 假定位域从最低有效位到最高有效位布局
// C++不保证这一点，但是编译器经常提供一种机制，允许程序员控制位域布局
struct IPv4Header {
    std::uint32_t version: 4,
                  IHL: 4,
                  DSCP: 6,
                  ECN: 2,
                  totalLength: 16;
};

int main() {
    {
        f({ 1, 2, 3 }); // 可以，{1, 2, 3}隐式转换为std::vector<int>
        // fwd({ 1, 2, 3 }); // 编译失败
        /*
        当通过调用函数模板fwd间接调用f时，编译器不再把调用地传入给fwd的实参和f的声明
        中形参类型进行比较。而是推导传入给fwd的实参类型，然后比较推导后的实参类型和f的形参声明类型。
        当下面情况任何一个发生时，完美转发就会失败：
        1. 编译器不能推导出fwd的一个或者多个形参类型。 这种情况下代码无法编译
        2. 编译器推导“错”了fwd的一个或者多个形参类型。
        上面的fwd({1, 2, 3})例子中，将花括号初始化传递给未声明为std::initializer_list的函数模板形参，
        被推导为“非推导上下文”。简单来说，这意味着编译器不准在对fwd的调用中推导表达式{1,2,3}的类型，因为
        fwd的形参没有声明为std::initializer_list。对于fwd形参的推导类型被阻止，编译器只能拒绝该调用。
        */
        // 但是可以修改成下面形式
        auto il = {1, 2, 3};
        fwd(il);
    }
    {    
        std::vector<int> widgetData;
        /*
        这里使用Widget::MinVals来确定widgetData的初始容量，即使MinVals缺少定义.
        编译器通过将值28放如入所有提到MinVals的位置来补充缺少的定义。
        没有为MinVals的值留存储空间是没有问题的，如果要使用MinVals的地址（例如，有人创建了指向MinVals的指针），
        则MinVals需要存储（这样指针才有可指向的东西），尽管上面的代码仍然可以编译，
        但是链接时就会报错，直到为MinVals提供定义。
        */
        widgetData.reserve(Widget::MinVals); // 使用MinVals
        f(Widget::MinVals);
        // 添加定义后正常执行
        // 引用“通常”被看作指针，需要定义，即存储地址
        fwd(Widget::MinVals); // error: undefined reference to `Widget::MinVals'
    }
    {
        /*
        f1要求一个函数指针作为实参，但是processVal不是一个函数指针或者一个函数，
        而是同名的两个不同函数。但是编译器知道它需要哪个：匹配上f的形参类型的那个。
        但是fwd是一个函数模板，没有它可以接受的类型的信息，使得编译器不可能决定出哪个函数应该被传递
        */
        f(processVal);
        // fwd(processVal); // error, 哪个processVal？
        // fwd(workOnVal); // error, 一个函数模板不代表单独一个函数，表示一个函数族
        using ProcessFuncType = int (*)(int); // 写个类型定义
        ProcessFuncType processValPtr = processVal; // 指定所需要的processVal签名
        fwd(processValPtr);
        fwd(static_cast<ProcessFuncType>(workOnVal)); 
    }
    {
        IPv4Header h;
        f(h.totalLength);
        // fwd(h.totalLength); // error
        std::cout << sizeof(h) << std::endl; // 4
        // 位域可能包含了机器字的任意部分（比如32位int的3-5位），但是这些东西无法直接寻址。
        // 一旦意识到接收位域实参的函数都将接收位域的副本，就可以轻松解决位域不能完美转发的问题。
        auto length = static_cast<u_int16_t>(h.totalLength);
        fwd(length);
    }
}