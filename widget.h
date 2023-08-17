#ifndef WIDGET_H
#define WIDGET_H
// 不使用Pimpl原则
// #include "gadent.h"

// class Widget {
// public:
//     Widget();
// private:
//     Gadent g;
// };

#include <memory>

// 使用Pimpl原则
class Widget {
public:
    Widget();
    /*
        如果Widget的析构/移动构造/移动赋值在Impl结构体还未定义处就定义了，会报错
        原因：在unique_ptr析构的时候，默认删除器会使用delete来销毁内置于unique_ptr的原始指针，
             然后再delete之前通常会使用c++11的特性static_assert来确保原始指针指向的类型不是一个未完成类型

        移动操作也同理，因为 pImpl = std::move(rhs.pImpl)也要先将其析构
    */
    ~Widget();
    Widget(Widget &&rhs);
    Widget& operator=(Widget && rhs);
    // Widget(Widget &&rhs) = default;
    // Widget& operator=(Widget &&rhs) = default;
private:
    /// @brief  声明，但是编译器不知道多大
    struct Impl;
    /// @brief 指针，固定大小，若不是指针，会报错，因为编译器不知道该结构体大小
    // Impl* pImpl;
    /// 修改指针样式
    // 若是使用shared_ptr，则不会存在unique_str的问题，因为它们的删除构造器有所不同
    std::unique_ptr<Impl> pImpl;
};
#endif