/*
item5: 优先考虑auto而非显式类型声明

*/

#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>

class Widget {
public:
    int operator <(const Widget &w) {
        return 1;
    }
};

template<typename It>
void dwim(It b, It e) {
    while(b != e) {
        typename std::iterator_traits<It>::value_type currValue = *b;
        auto curr = *b;
        std::cout << curr << " " << currValue << std::endl;
        b++;
    }
}

int main() {
    {
        // auto必须初始化
        int x1; // 潜在的未初始化变量
        // auto x2; // error
        auto x3 = 0;

        int a = 10;
        // float &b = a; // error
        const float &c = a;
        // 等价于 float tmp = a; const float &c = tmp;
    }
    {
        // C++11
        auto derefUPLess = [](const std::unique_ptr<Widget> &p1, 
        const std::unique_ptr<Widget> &p2) {
            return *p1 < *p2;
        };
    }
    {
        //C++14
        auto derefUPLess = [](const auto &p1, const auto &p2) {
            return *p1 < *p2;
        };
    }
    {
        /*
        std::function是C++11标准模板库中的一个模板，它泛化了函数指针的概念。
        std::function可以指向任何可调用对象，也就是哪些像函数一样能进行调用的东西。
        声明函数指针时必须指定函数类型（即函数签名）
        创建std::function对象时也需要提供函数签名，其是模板，所以需要在模板参数里面提供
        */
       // 声明了一个函数bool(const std::unique_ptr<Widget>&, const std::unique_ptr<Widget>);
       std::function<bool(const std::unique_ptr<Widget> &, 
       const std::unique_ptr<Widget> &)> func;
       // lambda表达式可以产生一个可调用对象
       std::function<bool(const std::unique_ptr<Widget> &, 
       const std::unique_ptr<Widget> &)>
       derefUPLess = [](const std::unique_ptr<Widget> &p1, 
       const std::unique_ptr<Widget> &p2) {
        return *p1 < *p2;
       };
       /*
       用auto声明的变量保存一个和闭包一样类型的（新）闭包，
       因此使用了与闭包相同大小存储空间。
       而实例化std::function并声明一个对象，这个对象将有固定大小，
       可能不足以存储一个闭包，这时会在堆上分配内存来存储。
       所以使用std::function比auto声明变量会消耗更多内存，且更慢
       （也可以使用std::bind生成一个闭包，详见item34）
       */
    }
    {
        /*
        auto除了可以避免未初始化的无效变量，省略冗长的声明类型，直接保存闭包外，
        还有一个好处是可以避免与类型快捷方式（type shortcuts）有关的问题
        */
       std::vector<int> v;
       // 在Windows32-bit上std::size_t大小与unsigned一样，32位
       // 在Windows64-bit上std::size_t是64位，而unsigned是32位
       unsigned sz = v.size();
       auto x = v.size();
    }
    {
        std::unordered_map<std::string, int> m;

        for(const std::pair<std::string, int>& p : m) {
            // 会拷贝m中的对象创建一个临时对象，然后把p的引用绑定到临时对象上
        }

        for(const auto &p : m) {

        }
    }
}