/*
item13：优先考虑const_iterator而非iterator

请记住
1. 优先考虑const_iterator而非iterator
2，在最大程度通用的代码中，优先考虑非成员函数版本的begin，end，rbegin等，而非同名成员函数

STL const_iterator等价于指向常量的指针，它们都指向不能被修改的值。
标准实践是能加上const就加上，这也指示我们需要一个迭代器时只要没必要
修改迭代器指向的值，就应当使用const_iterator

const_iterator在C++98中会有很多问题，不如它的兄弟（译注：指iterator）有用。
最终，开发者们不再相信能加const就加它的教条，而是只在实用的地方加它，
C++98的const_iterator不是那么实用。

而在C++11中，const_iterator既容易获取又容易使用。
容器的成员函数cbegin和cend产出const_iterator，甚至对于non-const容器也可用
那些之前使用iterator指示位置（如insert和erase）的STL成员函数也可以使用const_iterator了

唯一一个C++11对于const_iterator支持不足（C++14修正）的是：
当想要写最大程度通用的库，且这些库代码为一些容器和类似容器的数据结构提供begin、end(以及
cbegin，cend，rbegin，rend等)作为非成员函数而不是成员函数时。其中一种情况是原生数组，
还有一种情况是一些只由自由函数组成接口的第三方库。（自由函数free function，指的是非成员
函数，即一个函数只要不是成员函数，就可以被称为free function）最大程度通用的库会考虑使用非
成员函数而不是假设成员函数版本存在。加入findAndInsert函数模板
*/
#include <iostream>
#include <vector>
#include <algorithm>

// 在C++14工作良好，但是在C++11中不行。C++11只添加了非成员函数begin和end
// 而没有添加cbegin，cend，rbegin，rend，crbegin，crend
template<typename C, typename V>
void findAndInsert(C& container, const V& targetVal, const V& insertVal) {
    using std::cbegin;
    using std::cend;
    auto it = std::find(cbegin(container), cend(container), targetVal);
    container.insert(it, insertVal);
}
// 要想在C++11中使用，下面是非成员函数cbegin的实现
template<class C>
auto cbegin(const C& container) -> decltype(std::begin(container)) {
    // 如果C是一个普通的容器类型，如std::vector<int>
    // container将会引用一个const版本，const std::vector<int> &
    // 对const容器调用非成员函数begin（C++11提供）将产生const_iterator
    // 这个迭代器也是要返回的
    return std::begin(container);
}

int main() {
    {
        std::vector<int> values;
        for(int i = 0; i < 10; ++i) {
            values.push_back(i);
        }
        // 没有修改iterator指向的内容，使用const_iterator更好一些
        std::vector<int>::iterator it = std::find(values.begin(), values.end(), 5);
        values.insert(it, 1998);
        for(int x : values) {
            std::cout << x << " ";
        }
        std::cout << std::endl;
    }
    {
        // C++98中
        // std::vector<int> values;
        // for(int i = 0; i < 10; ++i) {
        //     values.push_back(i);
        // }
        // typedef std::vector<int>::iterator IterT;
        // typedef std::vector<int>::const_iterator ConstIterT;

        // C++98中values是non-const容器，没办法简单的从non-const容器中获取const_iterator
        // 严格来说类型转换不是必须的，其他方法也可以，比如可以把values绑定到reference-to-const变量上，
        // 然后使用该变量代替values，不管怎么说，从non-const容器中获取const_iterator的做法都优点别扭
        // ConstIterT ci = std::find(static_cast<ConstIterT>(values.begin()),
                                //   static_cast<ConstIterT>(values.end()),
                                //   5);
        // C++98中，插入操作（以及删除操作）的位置只能由iterator指定，const_iterator是不被接受的
        // 编译报错原因是 没有一个可移植的从const_iterator到iterator的方法，即使使用static_cast也不行
        // 甚至是reinterpret_cast也不行
        // values.insert(static_cast<IterT>(ci), 1998); // error 不能通过编译，类型转换错误
    }
    {
        // C++11
        std::vector<int> values;
        for(int i = 0; i < 10; ++i) {
            values.push_back(i);
        }
        // 没有修改iterator指向的内容，使用const_iterator更好一些
        auto it = std::find(values.cbegin(), values.cend(), 5);
        values.insert(it, 1998);
        for(int x : values) {
            std::cout << x << " ";
        }
        std::cout << std::endl;
    }
}