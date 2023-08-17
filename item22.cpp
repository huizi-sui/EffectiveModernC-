#include "widget.h"
#include <iostream>

/*
    条款22：当使用Pimpl惯用法时，请在实现文件中定义特殊成员函数
*/

int main() {
    Widget w;
    {
        Widget w2(std::move(w));
    }
    {
        int x = 10;
        for(int i = 0; i < x; ++i) {
            std::cout << "Hello i = " << i << std::endl;
        }
    }
}