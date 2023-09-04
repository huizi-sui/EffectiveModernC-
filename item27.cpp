/*
item27: 熟悉重载通用引用的替代品

1. 放弃重载
例如item26中的logAndAdd函数。但是对于类的构造函数来说是不现实的。
2. 传递const T&
退回C++98，将传递通用引用替换为传递lvalue-reference-to-const。这是首先考虑的方法，但是效率不高。
3. 传值
通常在不增加复杂性情况下提高性能的一种方法是，将按传引用形参替换为按值传递，这是违反直觉的。
该设计遵循item41中给出的建议
3. tag dispatch
这是模板元编程的标准构建模块
4. 约束使用通用引用的模板
编译器可能自行生成拷贝构造和移动构造函数，所以即使只写了一个构造函数并在其中使用tag dispatch，
有一些对构造函数的调用也被编译器生成的函数处理，绕过了分发机制。
需要使用std::enable_if，它提供一种强制编译器执行行为的方法，像是特定模板不存在一样。
这种模板被称为被禁止（disabled）。默认情况下，所有模板是启用的（enabled）。
使用std::enable_if可以使得仅在std::enable_if指定的条件满足时模板才启用。
class Person {
public:
    template<typename T, typename = typename std::enable_if<condition>::type>
    explicit Person(T&& n);
}

请记住：
1. 通用引用和重载的组合替代方案包括使用不同的函数名，
通过lvalue-reference-to-const传递形参，按值传递形参，使用tag dispatch。
2. 通过std::enable_if约束模板，允许组合通用引用和重载使用，
但它也控制了编译器在哪种条件下才使用通用引用重载。
3， 通用引用参数通常具有高效率的优势，但是可用性就值得斟酌。
*/
#include <iostream>
#include <set>
#include <type_traits>

std::multiset<std::string> names;

std::string nameFromIdx(int idx) {
    return std::to_string(idx);
}

class Person {
public:
    // 代替T&&构造函数
    explicit Person(std::string n) : name(std::move(n)) {
        std::cout << "string" << std::endl;
    } 
    // 同之前一样(item26)
    explicit Person(int idx) : name(nameFromIdx(idx)) {
        std::cout << "int" << std::endl;
    }
private:
    std::string name;
};

template<typename T>
void logAndAdd(T&& name);
/*
logAndAdd传递一个布尔值给logAndAddImpl表明是否传入了一个整型类型。
但是true和false是运行时值，需要重载决议——编译时决策——来选择正确的logAndAddImpl重载。
因此使用std::true_type和std::false_type
在这个设计中，类型std::true_type和std::false_type是标签（tag）
*/
// 第一个重载函数，用于处理非整型类型
template<typename T>
void logAndAddImpl(T&& name, std::false_type) {
    names.emplace(std::forward<T>(name));
}
// 第二个重载函数，用于处理整型
void logAndAddImpl(int idx, std::true_type c) {
    logAndAdd(nameFromIdx(idx));
}

template<typename T>
void logAndAdd(T&& name) {
    // 不那么正确
    // logAndAddImpl(std::forward<T>(name), std::is_integral<T>());
    // 如果左值实参传递给通用引用name，对T类型推导会得到左值引用
    // 所以如果左值int传入logAndAdd，T将会被推断为int&
    // 这意味着std::is_integral<T>对于任何左值实参返回false
    // 所以需要type trait，移除类型的引用说明符
    logAndAddImpl(std::forward<T>(name), 
                  std::is_integral<typename std::remove_reference<T>::type>());
    // C++14
    // logAndAddImpl(std::forward<T>(name), std::is_integral<std::remove_reference_t<T>>());
}

/*
示例：只在传递的类型不是Person使用Person的完美转发构造函数。
如果传递的类型是Person，要禁止完美转发构造函数(即让编译器忽略它)，因为这会让拷贝或者移动构造函数处理调用，
这是我们想要使用Person初始化另一个Person的初衷。
type trait可以确定两个对象类型是否相同(std::is_same)
所以需要的就是!std::is_same<Person, T>::value，但是要注意左值来初始化通用引用会推到成左值引用
所以查看T时，应该忽略：
1. 是否是个引用。
2. 是不是const或者volatile
type trait中提供了消除T的引用，const，volatile修饰符，std::decay
(它还可以将数组或者函数退化为指针)
!std::is_same<Person, typename std::decay<T>::type>::value
(这里std::decay前面的typename是必需的，因为std::decay<T>::type的类型取决于模板形参T)
*/
class Person1 {
public:
    template<
        typename T, 
        typename = typename std::enable_if<
                        !std::is_same<Person1, 
                                      typename std::decay<T>::type
                                      >::value
                    >::type
    >
    explicit Person1(T&& n) {

    }
};

class SpecialPerson : public Person {
public:
    // 拷贝构造函数，调用基类的拷贝构造函数
    SpecialPerson(const SpecialPerson& rhs) : Person(rhs) {

    }
    // 移动构造函数，调用基类的移动构造函数
    SpecialPerson(SpecialPerson&& rhs) : Person(std::move(rhs)) {

    }
};
/*
当拷贝或者移动一个SpecialPerson对象时，会调用基类对应的拷贝或移动构造函数。
SpecialPerson和Person类型不同，因此会调用完美转发构造函数。
type trait中也存在一个判断一个类型是否继承自另一个类型的函数，即std::is_base_of
std::is_base_of<T1, T2>是true表示T2派生自T1
std::is_base_of<T,T>总是true
所以使用is_base_of代替is_same即可
*/
// C++11版本
class Person2 {
public:
    template<
        typename T,
        typename = typename std::enable_if<
                        !std::is_base_of<Person2, 
                                         typename std::decay<T>::type
                                         >::value
                    >::type
    >
    explicit Person2(T&& n) {

    }
};
// C++14版本
// 使用std::enable_if和std::decay的别名模板来少写typename和::type
class Person3 {
public:
    template<
        typename T,
        typename = std::enable_if_t<
                        !std::is_base_of<Person3,
                                         std::decay_t<T>
                                        >::value
                    >
        >
    explicit Person3(T&& n)  {

    }
};

// C++14 完整示例
class Person4 {
public:
    template<
        typename T,
        typename = std::enable_if_t<
            !std::is_base_of<Person4, std::decay_t<T>>::value
            &&
            !std::is_integral<std::remove_reference_t<T>>::value
        >
    >
    explicit Person4(T&& n) : name(std::forward<T>()) {
        // 我们知道完美转发函数的通用引用形参要作为std::string的初始化容器，
        // 所以可以使用static_assert来确认它可以起作用。
        // std::is_constructible这个type trait执行编译时测试，确定一个类型的对象是否可以
        // 用另一种不同类型（或多个类型）的对象（或多个对象）来构造
        
        // 断言可以用T对象创建std::string
        static_assert(std::is_constructible<std::string, T>::value,
                      "Parameter n can't be used to construct a std::string");
    }

    explicit Person4(int idx) : name(nameFromIdx(idx)) {

    }

    // 拷贝、移动构造函数
private:
    std::string name;
};


int main() {

}