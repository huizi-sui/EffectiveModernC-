/*
item32: 使用初始化捕获来移动对象到闭包中

如果有一个只能被移动的对象(例如：std::unique_ptr或者std::future)要进入闭包里，在C++11中无法实现。
C++14支持将对象移动到闭包中。而在C++11中有很多方法可以实现近似的移动捕获

缺少移动捕获被认为是C++11的一个缺点。
初始化捕获(又名通用lambda捕获)，C++11捕获形式能做的所有事它几乎都能做，甚至能完成更多的功能。
你不能用初始化捕获表达的东西是默认捕获模式，但是Item31说明提醒你无论如何都应该远离默认捕获模式。

使用初始化捕获可以让你指定：
1. 从lambda生成的闭包类中的数据成员名字
2. 初始化该成员的表达式

请记住：

1. 使用C++14的初始化捕获将对象移动到闭包中。
2. 在C++11中，通过手写类或std::bind的方式来模拟初始化捕获。
*/
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
class Widget {
public:
    bool isValidated() const;
    bool isProcessed() const;
    bool isArchived() const;
private:  
};

// 如果是C++11.不支持C++14的初始化捕获
class IsValAndArch {
public:
    using DataType = std::unique_ptr<Widget>;
    explicit IsValAndArch(DataType&& ptr) : pw(std::move(ptr)) {

    }
    bool operator()() const {
        return pw->isValidated() && pw->isArchived();
    }
private:
    DataType pw;
};

/*
如果坚持要使用lambda，移动捕获可以在C++11中这样模拟：
1. 将要捕获的对象移动到由std::bind产生的函数对象中；
2. 将“被捕获的”对象的引用赋予给lambda
*/

int main() {
    {
        auto pw = std::make_unique<Widget>();
        // =左侧指定闭包类中数据成员的名称为pw
        // =右侧则是初始化表达式
        /*
        =左侧的作用域不同于右侧的作用域。左侧的作用域是闭包类，右侧的作用域和lambda
        定义所在的作用域相同。
        */
        auto func = [pw = std::move(pw)]() { // 使用std::move(pw)初始化闭包数据成员
            return pw->isValidated() && pw->isArchived();
        };
        // 若不需要修改Widget则可以直接初始化pw
        auto func1 = [pw = std::make_unique<Widget>()]() {
            return pw->isValidated() && pw->isArchived();
        };
    }
    {
        auto func = IsValAndArch(std::make_unique<Widget>());
    }
    {
        // 假设创建vector对象，放入一组适当的值后，移动到闭包中
        // C++14
        std::vector<double> data;
        // ...
        auto func = [data = std::move(data)]() {
            // use data
        };
        // C++11
        // 对于每个左值实参，bind对象中的对应对象都是复制构造的
        // 对于每个右值，它都是移动构造的
        // 这里，第二个实参std::move(data)是一个右值，因此将data移动构造到绑定对象中。
        // 这种移动构造是模仿移动捕获的关键
        auto func1 = std::bind([](const std::vector<double>& data) {
            // use data
        }, std::move(data));
        // 默认情况下，从lambda生成的闭包类中的operator()成员函数是const的。
        // 这具有在lambda主体内把闭包中的所有数据渲染为const的效果
        // 但是，bind对象内部的移动构造的data副本不是const，因此为了防止在lambda
        // 内修改该data副本，lambda形参应声明为reference-to-const。
        // 如果将lambda声明为mutable，则闭包类中的operator()将不会声明为const
        // 并且在lambda的形参声明中省略const也是合适的
        auto func2 = std::bind([](std::vector<double>& data) mutable {
            // use data
        }, std::move(data));

        auto func3 = std::bind([](const std::unique_ptr<Widget>& pw) {
            return pw->isValidated() && pw->isArchived();
        }, std::make_unique<Widget>());
    }
}