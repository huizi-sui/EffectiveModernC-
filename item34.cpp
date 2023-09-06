/*
item34: 考虑lambda而非std::bind

C++11中的std::bind是C++98的std::bind1st和std::bind2nd的后续
在C++11中，lambda几乎总是比std::bind更好的选择

从std::bind返回的函数对象称为bind对象。
优先lambda而不是std::bind的最重要原因是lambda更易读。

请记住：

1. 与使用std::bind相比，lambda更易读，更具表达力并且可能更高效。
2. 只有在C++11中，std::bind可能对实现移动捕获或绑定带有模板化函数调用运算符的对象时会很有用。
*/
#include <iostream>
#include <chrono>
#include <functional>

// 假设有一个设置警报器的函数
// 一个时间点的类型定义
using Time = std::chrono::steady_clock::time_point;
enum class Sound {
    Beep, Siren, Whistle
};
// 时间段的类型定义
using Duration = std::chrono::steady_clock::duration;
// 在时间t，使用s声音响铃时长d
void setAlarm(Time t, Sound s, Duration d) {

}
enum class Volume {
    Normal, Loud, LoudPlusPlus
};
// setAlarm重载时，出现新的问题
// void setAlarm(Time t, Sound s, Duration d, Volume v) {

// }
// 压缩等级
enum class CompLevl {
    Low, Normal, High
};
class Widget {

};
Widget compress(const Widget& w, CompLevl lev) {
    // 制作w的压缩副本
    // ...
    return w;
}

class PolyWidget {
public:
    template<typename T>
    void operator()(const T& param) {
    }
};

int main() {
    {
        // 假设需要设置一个小时后响30s的警报器，但是具体声音仍未确定
        auto setSoundL = [](Sound s) {
            using namespace std::chrono;
            setAlarm(steady_clock::now() + hours(1), s, seconds(30));
        };
        // C++14
        auto setSoundL14 = [](Sound s) {
            using namespace std::chrono;
            using namespace std::literals; // 对于C++14后缀

            setAlarm(steady_clock::now() + 1h, s, 30s);
        };

        setSoundL(Sound::Siren); // setAlarm函数体在这可以很好的内联
    }
    {
        // 第一次编写对std::bind调用。这里存在一个后续会修复的错误
        using namespace std::chrono;
        using namespace std::literals;
        using namespace std::placeholders;

        // 报错，编译器无法确定应将两个setAlarm中的哪一个传递给std::bind
        auto setSoundB = std::bind(setAlarm, 
                                   steady_clock::now() + 1h, // 不正确
                                   _1,
                                   30s);
        /*
        在lambda中，表达式stead_clock::now() + 1h显然是setAlarm的实参。
        调用setAlarm时将会对其进行计算。可以理解：我们希望在调用setAlarm后一小时响铃。
        但是在std::bind调用中，将stead_clock::now() + 1h作为实参传递给了std::bind，
        而不是setAlarm。这意味着将在调用std::bind时对表达式进行求值，并且该表达式产生的时间
        将存储在产生的bind对象中。结果就是警报器将被设置在调用std::bind后一小时发出声音，
        而不是在调用setAlarm一小时后发出。
        */
        // 要解决此问题，需要告诉std::bind推迟对表达式的求值，直到调用setAlarm为止
        // 方法是将对std::bind的第二个调用嵌套在第一个调用中
        auto setSoundB1 = std::bind(setAlarm,
                                    // std::plus尖括号没有指定任何类型，因为在C++14中，通常可以省略
                                    // 标准运算符模板的模板类型实参
                                    std::bind(std::plus<>(), std::bind(steady_clock::now), 1h),
                                    _1,
                                    30s);
        // 等效于C++11中的
        // auto setSoundB2 = std::bind(setAlarm, 
        //                             std::bind(std::plus<steady_clock::time_point>(),
        //                                      std::bind(steady_clock::now),
        //                                      hours(1)),
        //                             _1, 
        //                             seconds(30));
        // using SetAlarm3ParamType = void(*)(Time t, Sound s, Duration d);
        // auto setSoundB3 = std::bind(static_cast<SetAlarm3ParamType>(setAlarm), 
        //                             std::bind(std::plus<steady_clock::time_point>(),
        //                                         std::bind(steady_clock::now), 
        //                                         hours(1)), 
        //                             _1, 
        //                             seconds(30));
        // setSoundB3(Sound::Siren);
        
        setSoundB1(Sound::Siren); // setAlarm函数体在这不太可能内联
        setSoundB(Sound::Siren);
        // setSoundB2和setSound3编译报错，原因不明
        // 所以使用lambda比使用std::bind能生成更快的代码
    }
    {
        // 判断实参是否在最小值和最大值之间的结果
        int lowVal = 1;
        int highVal = 1000;
        // C++14
        auto betweenL14 = [lowVal, highVal](const auto& val) {
            return lowVal <= val && val <= highVal;
        };
        // 使用std::bind可以表达相同的内容，但是非常复杂
        // C++14
        auto betweenB14 = std::bind(std::logical_and<>(), 
                                    std::bind(std::less_equal<>(), lowVal, std::placeholders::_1),
                                    std::bind(std::less_equal<>(), std::placeholders::_1, highVal));
        // C++11, 必须指定类型
        auto betweenB11 = std::bind(std::logical_and<bool>(), 
                                    std::bind(std::less_equal<int>(), lowVal, std::placeholders::_1), 
                                    std::bind(std::less_equal<int>(), std::placeholders::_1, highVal));
        // C++11, lambda不能使用auto形参，必须指定类型
        auto betweenL11 = [lowVal, highVal](const int val) {
            return lowVal <= val && val <= highVal;
        };
    }
    {
        // 使用bind
        Widget w;
        using namespace std::placeholders;
        auto compressRateB = std::bind(compress, w, _1);
        // 现在将w传递给std::bind时，必须将其存储起来，以便以后进行压缩。
        // 它存储在对象compressRateB中，但是它是通过值还是引用被存储的？
        // 在对std::bind的调用和compressRateB的调用之间修改了w，则按引用
        // 捕获的w将反映这个更改，而按值捕获则不会。
        // 答案是它是按值捕获的。std::bind总是拷贝它的实参
        // 若要使用引用来存储实参，需要使用std::ref
        auto compressRateB1 = std::bind(compress, std::ref(w), _1);
        // 使用lambda，w是通过值还是引用捕获是显式的
        auto compressRateL = [w](CompLevl lev) { // w按值捕获，lev按值传递
            return compress(w, lev);
        };
        compressRateL(CompLevl::High); // 按值传递
        // 唯一的方法是记住std::bind的工作方式
        // 传递给bind对象的所有实参都是通过引用传递的，因为此类对象的函数调用
        // 运算符使用完美转发
        compressRateB(CompLevl::High); // 实参如何传递？
    }
    {
        // 在C++14中，没有std::bind的合理用例。
        /*
         在C++11中，可以在两个受约束的情况下证明使用std::bind是合理的：
         1. 移动捕获。C++11的lambda不提供移动捕获，但是可以通过结合lambda和std::bind来模拟。
         2. 多态函数对象。因为bind对象上的函数调用运算符使用完美转发，所以它可以接受任何类型的实参。
         当要绑定带有模板化调用运算符的对象时，此功能很有用
        */
        using namespace std::placeholders;
        PolyWidget pw;
        auto boundPW = std::bind(pw, _1);
        boundPW(1930);
        boundPW(nullptr);
        boundPW("Hello world");
        // 这一点无法使用C++11做到
        // 在C++14中，可以通过带有auto形参的lambda轻松实现
        auto boundPWL = [pw](const auto& param) {
            pw(param);
        };
    }
}