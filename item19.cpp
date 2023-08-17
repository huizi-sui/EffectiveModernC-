#include <iostream>
#include <vector>
#include <memory>

/*
    对于共享资源使用std::shared_ptr
    std::unique_ptr默认是8字节，自定义删除器的设计会影响该指针的大小
    std::shared_ptr永远是16个字节，包括两个指针，一个指向目标object，另一个指向control block
    这两个都是在堆上生成
    control block包含三项，分别是reference count， week count， other data

    shared_ptr的性能问题
    1. 大小是原始指针的2倍
    2. 引用计数内存必须动态分配
    3. 引用计数的增减必须是原子的

    control block的生成时机
    1. 使用std::make_shared
    2. 通过unique_ptr构造shared_ptr
    3. 向shared_ptr的构造函数中传入一个裸指针

    对于shared_ptr,指向同一个对象时，它们使用的control block应当也是一样的，否则可能会出现重复释放的风险

*/
class Test {
public:
    Test(int a) {
        std::cout << "Test(int a)" << std::endl;
        this->a = a;
    }
    ~Test() {
        std::cout << "~Test()" << std::endl;
    }
private:
    int a;
};

class Widget;
std::vector<std::shared_ptr<Widget>> processWidgets;
/// @brief 这种方式会报错重复释放指针内存
// class Widget {
// public:
//     void process() {
//         processWidgets.emplace_back(this);
//     }
// };

// class Widget : public std::enable_shared_from_this<Widget> {
// public:
//     void process() {
//         processWidgets.emplace_back(shared_from_this());
//     }
// };

// 最终版本，禁止栈上创建
class Widget : public std::enable_shared_from_this<Widget> {
public:
    template<typename... Ts>
    static std::shared_ptr<Widget> create(Ts&& ...params) {
        return std::shared_ptr<Widget>(new Widget(std::forward<Ts>(params)...));
    }
    void process() {
        processWidgets.emplace_back(shared_from_this());
    }
private:
    Widget(int data) : _data(data) {}
    int _data;
};

int main() {
    {
        Test* pt = new Test(2);
        std::shared_ptr<Test> spt1(pt);
        // std::shared_ptr<Test> spt2(pt); // error: double free
        std::shared_ptr<Test> spt2(spt1);
    }
    {
        // 使用this指针作为std::shared_ptr构造函数实参
        // auto w = std::make_shared<Widget>();
        auto w = Widget::create(1);
        w->process();
    }
    // shared_ptr 不支持数组
    return 0;
}