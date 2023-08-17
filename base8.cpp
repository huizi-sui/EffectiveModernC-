/*

闭包： 带有上下文（状态）的函数，
闭包实现的3种方式：重载operator(); lambda表达式；std::bind

可调用对象类型
1. 函数
2. 函数指针
3. lambda表达式
4. 重载函数运算符的类
5. std::bind
*/
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

bool isShort(const std::string &s, int sz) {
    return s.size() >= sz;
}

int main() {
    std::vector<std::string> wordVec{"fun", "hello", "world"};

    int sz = 4;
    auto wc = std::find_if(wordVec.begin(), wordVec.end(), [sz](const std::string &s) {
        return s.size() >= sz;
    });

    using namespace std::placeholders;
    auto func = std::bind(isShort, _1, sz);
    auto wc1 = std::find_if(wordVec.begin(), wordVec.end(), func);
}
