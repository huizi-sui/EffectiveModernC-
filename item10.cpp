/*
item10：优先考虑限域枚举而非未限域枚举

请记住：
1. C++98的enum即非限域enum。
2. 限域enum的枚举名仅在enum内可见。要转换为其它类型只能使用cast。
3. 非限域/限域enum都支持底层类型说明语法，限域enum底层类型默认是int。非限域enum没有默认底层类型。
4. 限域enum总是可以前置声明。非限域enum仅当指定它们的底层类型时才能前置。

通常来说，在花括号中声明一个名字会限制它的作用域在花括号内。
对于C++98来说，enum不成立。这些枚举名的名字属于包含这个enum的作用域，
这意味着作用域内不能含有相同名字的其他东西

使用限域枚举可以减少命名空间的污染
第二个优点： 在它的作用域中，枚举名是强类型
而未限域enum中的枚举名会隐式转换为整型（现在也可以转换为浮点类型）
第三个好处： 限域enum可以前置声明
在C++11中，非限域enum也可以被前置声明，但是只有在做一些其他工作后才能实现。
这些工作来源于一个事实：在C++中所有的enum都有一个由编译器决定的整型的底层类型
对于非限域enum比如Color，
enum Color { black, white, red };
编译器可能选择char作为底层类型，因为这里只需要表示三个值。
而
enum Status {
    good = 0,
    failed = 1,
    incomplete = 100,
    corrupt = 200,
    indeterminate = 0xFFFFFFFF
};
这里值的范围从0到0xFFFFFFFFF，编译器会选一个比char大的整型类型来表示Status

不能前置声明的enum最大的缺点是它可能增加编译依赖
上面Status可能用于整个系统，因此系统中每个包含这个头文件的组件都会依赖它
如果引入一个新的状态值
enum Status {
    good = 0,
    failed = 1,
    incomplete = 100,
    corrupt = 200,
    audited = 500,
    indeterminate = 0xFFFFFFFF
};
那么可能整个系统都得重新编译，这是不希望看到的
C++11中的前置声明enum可以解决这个问题。
例如：
enum class Status; // 前置声明
void continueProcessing(Status s); //使用前置声明enum

即使Status的定义发生改变，包含这些声明的头文件不需要重新编译
如果Status有改动(比如添加一个auidited枚举名)，continueProcessing的行为
不受影响

如果编译器在使用它之前需要知晓enum的大小，怎么声明才能让C++11做到C++98不能
做到的事情？
答案： 限域enum的底层类型总是已知的，而对于非限域enum，可以指定它。
默认情况下，限域枚举的底层类型是int
enum class Status; // 底层类型是int
重写：
enum class Status : std::uint32_t;
要为非限域enum指定底层类型，同上，结果就可以前向声明
enum Color : std::uint8_t;

至少由一种情况下非限域enum很有用，就是牵扯到C++11的std::tuple时
*/

#include <iostream>
#include <vector>
#include <tuple>
#include <type_traits>

// 前置声明
// enum Color;  // error
enum class Color;

enum class Status; // 底层类型是int

enum class Status1 : std::uint32_t; // 底层类型是uint32_t

enum Color1 : std::uint8_t; // 非限域enum前向声明，底层类型是std::uint8_t
// 底层类型说明也可以放到enum定义处
enum class Status2 : std::uint32_t {
    good = 0,
    // ...
};


std::vector<std::size_t> primeFactors(std::size_t x) {
    std::vector<std::size_t> res;
    return res;
};

template<typename E>
constexpr typename std::underlying_type<E>::type
toUType(E enumerator) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(enumerator);
}
// 上面是C++11，在C++14中还可以进一步打磨
template<typename E>
constexpr std::underlying_type_t<E>
toUType1(E enumerator) noexcept {
    return static_cast<std::underlying_type_t<E>>(enumerator);
}
// 上面还可以再用C++14 atuo打磨
template<typename E>
constexpr auto toUType2(E enumerator) noexcept {
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

int main() {
    {
        // black,white,red在该花括号作用域
        // 未限域枚举
        enum Color {
            black, white, red
        };
        // bool white = false; // error: white早已经在该作用域声明
    }
    {
        // C++11中出现了限域枚举，它不会导致枚举名泄漏
        // 又称为枚举类
        enum class Color {
            black, white, red // 限制在Color域
        };
        bool white = false;

        // Color c = white; // error: 域中没有枚举名叫white
        Color c = Color::white;
        auto c1 = Color::white;
    }
    {
        enum Color { black, white, red };

        Color c = red;

        if(c < 14.5) { // Color与double比较 !
            auto factors = primeFactors(c); // 计算一个Color的质因子 !
        }
    }
    {
        enum class Color {
            black, white, red
        };
        Color c = Color::red;

        // if(c < 14.5) { // error: 不能比较Color与double
        //     auto factors = primeFactors(c); // error: 不能向参数为std::size_t的函数传递Color参数
        // }
        // 如果想要实现上面内容，需要类型转换
        if((double)c < 14.5) {
            auto factors = primeFactors((std::size_t)c);
        }
        // 或者是
        if(static_cast<double>(c) < 14.5) {
            auto factors = primeFactors(static_cast<std::size_t>(c));
        }
    }
    {
        // 类别别名
        using UserInfo = std::tuple<std::string, // 名字
                                    std::string, // emial地址
                                    std::size_t>; // 声望
        // 在另一个文件中遇到下面代码
        UserInfo uInfo; // tuple对象
        auto val = std::get<1>(uInfo); // 获得第一个字段
        // 应该记住第一个字段代表用户的emial地址么？  不！！！
        // 可以使用非限制enum将名字和字段编号关联起来以避免上述需求
        enum UserInfoFields {
            uiName, uiEmail, uiReputation 
        };
        UserInfo uInfo1;
        // 正常工作原因： UserInfoFields中的枚举名隐式转换成std::size_t了
        // 其中std::size_t是std::get模板实参所需要的
        auto val1 = std::get<uiEmail>(uInfo1); // 获取用户email字段的值

        // 对应的限域enum就很啰嗦了
        {
            enum class UserInfoFields {
                uiName, uiEmail, uiReputation
            };
            UserInfo uInfo;
            auto val = std::get<static_cast<std::size_t>(UserInfoFields::uiEmail)>(uInfo);
            /*
            为了避免这种冗长的表示，可以写一个函数传入枚举名并返回对应的std::size_t值
            但是std::get是一个模板函数，需要给出一个std::size_t值得模板实参，
            因此将枚举名变换为std::size_t值得函数必须在！！编译期！！产生这个结果。
            如Item15提到的，那必须是一个constexpr函数

            事实上，它也的确应该是一个constexpr函数模板，因为它应该能用于任何enum。
            如果想让它更一般化，还需要泛化它的返回类型。
            较之于std::size_t,我们更应该返回枚举的底层类型
            这可以通过std::undelying_type这个type trait获得。
            最终还需要加上noexcept修饰（参见item14： 如果函数不抛出异常请使用noexcept）
            因为我们知道它肯定不会产生异常。
            实现在上面的toUType函数模板
            */
           // 现在toUType现在允许这样访问tuple的字段了
           auto val1 = std::get<toUType(UserInfoFields::uiEmail)>(uInfo);
           auto val2 = std::get<toUType1(UserInfoFields::uiEmail)>(uInfo);
           auto val3 = std::get<toUType2(UserInfoFields::uiEmail)>(uInfo);
           /*
           这仍然比使用非限域enum要写更多的代码，但同时它也避免命名空间污染，防止不经意间使用隐式转换。
           大多数情况下，你应该会觉得多敲几个（几行）字符作为避免使用未限域枚举这种老得和2400波特率猫同时代技术的代价是值得的。
           */
        }
    }
}