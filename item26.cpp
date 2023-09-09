/*
item26: 避免在通用引用上重载

使用通用引用的函数在C++中是最贪婪的函数，它们几乎可以精确匹配任何类型的实参。

请记住：

1. 对通用引用形参的函数进行重载，通用引用函数的调用机会几乎总会比你期望的多得多。
2. 完美转发构造函数是糟糕的实现，因为对于non-const左值，它们比拷贝构造函数而更匹配，
而且会劫持派生类对于基类的拷贝和移动构造函数的调用。
*/
#include <iostream>
#include <set>

std::multiset<std::string> names; // 全局数据结构
void logAndAdd(const std::string& name) {
    names.emplace(name);
}

template<typename T>
void logAndAdd1(T&& name) {
    names.emplace(std::forward<T>(name));
}

std::string nameFromIdx(int idx) {
    return std::to_string(idx);
}
// 新的重载
void logAndAdd1(int idx) {
    names.emplace(nameFromIdx(idx));
}

class Person {
public:
    // 完美转发的构造函数，初始化数据成员
    template<typename T>
    explicit Person(T&& n) : name(std::forward<T>(n)) {

    }
    // int的构造函数
    explicit Person(int idx) : name(nameFromIdx(idx)) {

    }
    // 编译器会自动生成拷贝构造函数和移动构造函数
    // Person(const Person& rhs);
    // Person(Person&& rhs);
private:
    std::string name;
};

// 考虑继承时，完美转发的构造函数和编译器生成的拷贝、移动操作之间的交互更加复杂
// class SpecialPerson : public Person {
// public:
//     // 派生类将SpecialPerson类型的实参传递给其基类，然后通过模板实例化
//     // 和重载解析规则作用于基类Person。最终代码无法编译，因为std::string
//     // 没有接受一个SpecialPerson的构造函数
//     SpecialPerson(const SpecialPerson& rhs) : Person(rhs) {
//         // 拷贝构造函数，调用基类的完美转发构造函数
//     }
//     SpecialPerson(SpecialPerson&& rhs) : Person(std::move(rhs)) {
//         // 移动构造函数，调用基类的完美转发构造函数
//     }
// };

int main() {
    {
        std::string petName("Darla");
        /*
        logAndAdd形参name绑定到变量petName上。然后传给names.emplace
        因为name是左值，会拷贝到names中。
        */
        logAndAdd(petName); // 传递左值std::string
        /*
        形参name绑定到右值（显式从"Persephone"创建的临时对象std::string）
        name本身是个左值，所以它被拷贝到names中。
        但是可以意识到，原则上，它的值可以被移动到names中。
        本次调用中，有个拷贝代价，但是应该能用移动勉强应付
        */
        logAndAdd(std::string("Persephone")); // 传递右值std::string
        /*
        形参name也绑定了一个右值，但是这次是通过"Patty Dog"隐式创建的临时string
        变量。就像第二个调用中，name被拷贝到names
        如果直接将字符串字面量传递给emplace，就不会创建std::string的临时对象
        而是在std::multiset中通过字面量构建std::string。
        在这里，有一个string拷贝开销
        */
        logAndAdd("Patty Dog"); // 传递字符串字面值
    }
    // 使用通用引用重写logAndAdd函数，提升第二个和第三个调用效率
    {
        std::string petName("Darla");
        logAndAdd1(petName); // 拷贝左值到multiset
        logAndAdd1(std::string("Persephone")); // 移动右值而不是拷贝它
        logAndAdd1("Patty Dog"); // 在mutiset直接创建std::string，而不是拷贝一个临时std::string

    }
    {
        std::string petName("Darla");
        logAndAdd1(petName); // 拷贝左值到multiset
        logAndAdd1(std::string("Persephone")); // 移动右值而不是拷贝它
        logAndAdd1("Patty Dog"); // 在mutiset直接创建std::string，而不是拷贝一个临时std::string

        logAndAdd1(22); // 调用int重载版本

        short nameIdx = 1;
        /*
        使用通用引用的函数推导T的类型为short，因此精确匹配。
        对于int类型参数的重载则在short类型提升后匹配成功。
        而精确匹配优先于类型提升的匹配，因此调用的是通用引用的重载。

        name形参绑定到传入的short上，然后name被std::forward给names的emplace成员函数，
        然后又被转发给std::string构造函数。std::string没有接受short的构造函数，
        所以调用失败。
        */
        // logAndAdd1(nameIdx); // error
    }
    {
        Person p("Nancy");
        /*
        试图通过Person实例p创建另一个Person对象cloneOfP，显然应该调用拷贝构造函数。
        但是这里不是调用拷贝构造函数，而是调用完美转发构造函数。 完美转发函数
        将尝试使用Person对象p初始化Person的std::string数据成员，编译器就会报错。
        因为编译器会严格遵循C++的规则，这里就是控制对重载函数调用的解析规则。
        编译器理由如下：cloneOfP被non-const左值p初始化，因为这模板化构造函数
        可以被实例化为采用Person类型的non-const左值。实例化之后，Person类如下：
        class Person {
        public:
            explicit Person(Person& n) : name(std::forward<Person&>(n)) {}
            explicit Person(int idx) {
                ...
            }
            Person(const Person& rhs); // 编译器生成的拷贝构造函数
        };
        所以下面语句中，调用拷贝构造函数要求在p前加上const的约束来满足函数形参的类型，
        而调用完美构造不需要加这些东西。所以编译器会选择完美转发构造函数。
        */
        // auto cloneOfP(p); // error 调用的是完美转发函数模板
        const Person cp("Nancy"); // 现在对象是const
        /*
        虽然这时函数模板会实例化为下面形式：
        explicit Person(const Person& n);
        Person(const Person& rhs); // 编译器生成的拷贝构造函数
        但是没有影响，因为重载规则规定当模板实例化函数和非模板函数匹配优先级
        相当时，有限使用非模板函数。
        */
        auto cloneOfP(cp); // 调用拷贝构造函数
    }
}