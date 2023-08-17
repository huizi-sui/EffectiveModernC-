#include <iostream>
#include <memory>
#include <unordered_map>

/*
    item21内容： 尽量使用make_shared和make_uniqe，而不是std::unique_str(new Widget());

*/

class Widget {
public:
    Widget(int _id) : id(_id) {
        std::cout << "Widget()" << std::endl; 
    }
    ~Widget() {
        std::cout << "~Widget()" << std::endl;
    }
private:
    int id;
};

std::unique_ptr<const Widget> loadWidget(int id) {
    // 耗时操作
    std::unique_ptr<const Widget> uptr(new Widget(id));
    return uptr;
}

std::shared_ptr<const Widget> fastLoadWidget(int id) {
    static std::unordered_map<int, std::weak_ptr<const Widget>> cache;
    auto objPtr = cache[id].lock();
    if(!objPtr) {
        objPtr = loadWidget(id);
        cache[id] = objPtr;
    }
    return objPtr;
}

// weak_ptr作用： 监视者
int main() {
    {
        // weak_ptr不能单独使用，通常从shared_ptr上创建
        auto spw = std::shared_ptr<Widget>(new Widget(1));
        if(spw.get() == nullptr) {
            std::cout << "spw is null" << std::endl;
        }
        std::weak_ptr<Widget> wpw(spw);
        spw = nullptr; // 释放堆内存
        std::cout << "over" << std::endl;
        // weak_ptr不会影响到shared_ptr的生存周期，但是能够检测到
        // 1. wpw.expired(); // 返回true表明资源已经释放
        // 2. std::shared_ptr<Widget> spw1 = wpw.lock(); // 如果wpw过期，则spw1为nullptr
        // 3. std::shared_ptr<Widget> spw2(wpw); // 抛异常
    }

    {
        auto widgetSPtr = fastLoadWidget(0);
        // 这里，如果fastLoadWidget()函数中的哈希存的是shared_ptr，那堆上的内存不会得到释放
        widgetSPtr = fastLoadWidget(1);
        std::cout << "over" << std::endl;
    }
}