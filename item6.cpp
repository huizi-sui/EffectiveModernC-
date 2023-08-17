/*
item6: auto推导若非己原，使用显式类型初始化惯用法

*/
#include <iostream>
#include <vector>

class Widget {

};

std::vector<bool> features(const Widget& w) {
    std::vector<bool> v(10, true);
    return v;
}

void processWidget(Widget &w, bool priority) {
    std::cout << priority << std::endl;
}

int main() {
    {
        Widget w;
        // std::vector<bool>使用打包形式表示它的bool
        // 每个bool占一个bit
        // 这里会进行隐式转换，将std::vector<bool>::reference隐式转换为bool
        bool highPriority = features(w)[5];
        processWidget(w, highPriority);

        // 使用auto不再是bool类型，
        // 而是std::vector<bool>::reference的对象（一个嵌套于std::vector<bool>中的类）
        auto highPriority1 = features(w)[5];
        processWidget(w, highPriority1); // 未定义行为

        auto highPriority2 = static_cast<bool>(features(w)[5]);
    }
    std::cout << "item6" << std::endl;
}