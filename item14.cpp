/*
item14：如果函数不抛出异常请使用noexcept

请记住：
1. noexcept是函数接口的一部分，这意味着调用者可能会依赖它
2. noexcept函数较之于non-noexcept函数更容易优化
3. noexcept对于移动语义，swap，内存释放函数和析构函数非常有用
4. 大多数函数是异常中立的（译注：可能抛也可能不抛异常）而不是noexcept

异常说明真正有用的信息是一个函数是否会抛出异常，非黑即白
一个函数可能抛异常，或者不会
这中“可能-绝不”的二元论构成了C++11异常说的基础，从根本上改变了C++98的异常说明
（C++98风格的异常说明也有效，但是以及标记为deprecated(废弃)）
在C++11中，无条件的noexcept保证函数不会抛出任何异常

函数是否为noexpect和成员函数是否const一样重要

给不抛异常的函数加上noexcept的动机：它允许编译器生成更好的目标代码
*/
#include <iostream>
/*
在运行时，如果f出现异常，那么就和f的异常说明冲突了。
在C++98的异常说明中，调用栈会展开至f的调用者，在一些与这地方不相关的工作后，程序被终止。
C++11异常说明的运行时行为有些不同：调用栈只是可能在程序终止前展开。
展开调用栈和可能展开调用栈两者对于代码生成（code generation）有非常大的影响。
在一个noexcept函数中，当异常可能传播到函数外时，优化器不需要保证运行时栈（the runtime stack）处于可展开状态；
也不需要保证当异常离开noexcept函数时，noexcept函数中的对象按照构造的反序析构。
而标注“throw()”异常声明的函数缺少这样的优化灵活性，没加异常声明的函数也一样。
总结如下：
RetType function(params) noexcept; // 极尽所能优化
RetType function(params) throw(); // 较少优化
RetType function(params); // 较少优化
*/
int f(int x) throw() {
    // C++98风格，没有来自f的异常
}

int f1(int x) noexcept {
    // C++11风格，没有来自f的异常
}

int main() {
    
}