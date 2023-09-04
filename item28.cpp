/*
item28: 理解引用折叠

存在两种类型的引用（左值和右值），所以有四种可能的引用组合（左值的左值，
左值的右值，右值的右值，右值的左值）
如果一个上下文允许引用的引用存在（比如，模板的实例化），引用根据规则折叠为单个引用：
如果任一引用为左值引用，则结果为左值引用。否则（即，如果引用都是右值引用），结果为右值引用。

所以通用引用不是一种新的引用，实际上是满足以下两个条件的右值引用：
1. 类型推导区分左值和右值。 T类型的左值被推导为T&类型，T类型的右值被推导为T。
2. 发生引用折叠。

请记住：
1. 引用折叠发生在四种情况下：模板实例化，auto类型推导，typedef与别名声明的创建和使用，decltype。
2. 当编译器在引用折叠环境中生成了引用的引用时，结果就是单个引用。有左值引用折叠结果就是左值引用，否则就是右值引用。
3. 通用引用就是在特定上下文的右值引用，上下文是通过类型推导区分左值还是右值，并且发生引用折叠的那些地方。
*/
#include <iostream>
#include <type_traits>

class Widget {

};

template<typename T>
void func(T&& param) {

}

Widget widgetFactory() {
    Widget w;
    return w;
}

template<typename T>
void someFunc(T&& param) {

}

// 引用折叠是std::forward工作的一种关键机制
// std::forward应用在通用引用参数上
template<typename T>
void f(T&& param) {
    // ... 做些工作
    someFunc(std::forward<T>(param)); // 转发param到someFunc
    /*
    因为param是通用引用，所以类型参数T的类型根据f被传入实参是左值还是右值来编码
    std::forward的作用是当且仅当传给f的实参是右值时，即T为非引用类型，才将param（左值）转换为一个右值。
    */
}

// std::forward实现类似下面函数，忽略一些接口描述。
/*
假设传入f的实参是Widget左值类型。T被推导为Widget&，然后调用std::forward将实例化为
std::forward<Widget&>。变为
Widget& && forward1(typename std::remove_reference<Widget&>::type& param) {
    return static_cast<Widget& &&>(param);
}
remove_reference将会使Widget&变为Widget，因此forward1变为
Widget& && forward1(Widget& param) {
    return static_cast<Widget& &&>(param);
}
最后根据引用折叠规则，返回值和强制转化可以化简。
Widget& forward1(Widget& param) {
    return static_cast<Widget&>(param);
}
所以没有任何变化
*/
/*
假设传递给f的实参是一个Widget的右值。f的类型参数T的推导类型就是Widget
然后调用std::forward<Widget>, 模板实例化得到
Widget&& forward1(typename std::remove_reference<Widget>::type& param) {
    return static_cast<Widget&&>(param);
}
remove_reference引用到非引用类型Widget上还是相同的类型Widget，变为
Widget&& forward1(Widget& param) {
    return static_cast<Widget&&>(param);
}
*/
template<typename T>
T&& forward1(typename std::remove_reference<T>::type& param) {
    return static_cast<T&&>(param);
}
// C++14中
template<typename T>
T&& forward2(std::remove_reference_t<T>& param) {
    return static_cast<T&&>(param);
}

template<typename T>
class Widget1 {
public:
    typedef T&& RvalueRefToT;
};

int main() {
    {
        Widget w; 
        func(w); // 用左值调用func，T被推导为Widget&
        func(widgetFactory()); // 用右值调用func，T被推导为Widget
    }
    {
        int x;
        // auto& & rx = x; // error 不能声明引用的引用
        /*
        传递左值给通用引用的模板函数
        T会被推导为Widget&
        因此初始化模板得到：
        void func(Widget& && param); 
        引用的引用，但是编译器没有报错。
        因为编译器使用了引用折叠。
        上面将推导类型Widget&替换进模板func会产生对左值引用的右值引用，然后引用折叠规则
        告诉我们结果就是左值引用。
        */
    }
    {
        /*
        引用折叠发生在四种情况下。
        1. 也是最常见的，就是模板实例化。
        2. auto变量的类型生成，具体细节类似于模板。
        3. typedef和别名声明的产生和使用中。
        4. decltype的使用。在分析decltype期间，出现了引用的引用，引用折叠规则就会起作用
        */
       Widget w;
       auto&& w1 = w; // 用左值初始化w1，所以auto推导出类型为Widget&
       // 得到Widget& && w1 = w; ===> Widget& w1 = w; 引用折叠
       auto&& w2 = widgetFactory(); // 使用右值初始化，为auto推导出非引用类型Widget，
       // 得到Widget&& w2 = widgetFactory()

       // 假设使用左值引用实例化Widget1
       Widget1<int&> w3;
       // Widget1模板中把T替换为int&得到typedef int& && RvalueRefToT;
       // 引用折叠就会发挥作用，typedef int& RvalueRefToT;
       // 这表明为itypedef选择的名字可能不是我们希望的那样：当使用左值引用类型实例化Widget1时，
       // RvalueRefToT是左值引用的typedef
    }
}