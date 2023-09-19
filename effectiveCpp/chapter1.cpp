#include <iostream>
#include <vector>
#include <cstring>
/*
item1: 视C++为一个语言联邦
C++拥有多种不同的编程范式，而这些范式集成在一个语言中，
使得C++是一门既灵活又复杂的语言：
1. 传统的面向过程C：区块，语句，预处理器，内置数据类似、数组、指针
2. 面向对象的C with Classes：类、封装、继承、多态、动态绑定
3. 模板编程Template C++和堪称黑魔法的模板元编程(TMP)
4. C++标准库STL
*/
/*
item2：尽量以const，enum，inline替换#define
在C++11中出现constexpr，现在一般认为应当用constexpr定义
编译期常量来替代大部分的#define宏常量定义
*/
#define ASPECT_RATIO 1.653
// 替换为
constexpr auto aspect_ratio = 1.653;
// 也可以讲编译期常量定义为类的静态成员
class GamePlayer {
public:
    static constexpr auto numTurns = 5;
};
// enum可以用来替代整型的常量，并且在模板元编程中应用广泛
class GamePlayer1 {
public:
    enum { numTurns = 5 };
};
// 大部分#define宏常量应当用内联模板函数替代
#define CALL_WITH_MAX(a, b) f((a) > (b) ? (a) : (b))
// 替代为
template<typename T>
inline void CallWithMax(const T& a, const T& b) {
    f(a > b ? a : b);
}
// 注意：宏和函数的行为本身不完全一致，宏只是简单的替换，不涉及传参和复制

/*
item3：尽可能使用const
若想让一个常量只读，应该明确说出它是const常量，对于指针来说更是如此
*/
char greeting[] = "hello";
char* p = greeting; // 指针可修改，数据可修改
const char* p1 = greeting; // 指针可修改，数据不可修改
char const* p2 = greeting; // 指针可修改，数据不可修改
char* const p3 = greeting; // 指针不可修改，数据可修改
const char* const p4 = greeting; // 指针不可修改，数据不可修改
// 对于STL迭代器，愤青使用const还是const_iterator
std::vector<int> vec;
const std::vector<int>::iterator iter = vec.begin(); // 迭代器不可修改，数据可修改
std::vector<int>::const_iterator const_iter = vec.cbegin(); // 迭代器可修改，数据不可修改
// 面对函数声明时，如果不想让一个函数的结果被无意义地当作左值
// (也就是不能被修改)，使用const返回值
class Rational {};
const Rational operator*(const Rational& lhs, const Rational& rhs) {}
// const成员函数
// const成员函数允许我们操控const对象，在传递常引用时显得尤为重要
class TextBlock {
public:
    // const对象使用的重载
    const char& operator[](std::size_t position) const {
        return text[position];
    }
    // non-const对象使用的重载
    char& operator[](std::size_t position) {
        return text[position];
    }
private:
    std::string text;
};
void Print(const TextBlock& ctb) {
    std::cout << ctb[0]; // 调用const TextBlock::operator[]
}
// 编译器对待const对象的态度通常是bitwise constness，而我们编写程序时
// 通常采用logical constness，这意味着在确保客户端不会察觉的情况下，
// 我们认为const对象中的某些成员变量应当是允许被改变的，使用关键字
// mutable来标记这些成员变量
class CTextBlock {
public:
    std::size_t Length() const;
private:
    char* pText;
    mutable std::size_t textLength;
    mutable bool lengthVaild;
};
std::size_t CTextBlock::Length() const {
    if(!lengthVaild) {
        textLength = std::strlen(pText); // 可以修改mutable成员变量
        lengthVaild = true; // 可以修改mutable成员变量
    }
    return textLength;
}
// 在重载const和non-const成员函数时，需要尽可能避免书写重复的内容，
// 这促使我们去进行常量性转除。在大部分情况下，应当避免转型的出现，
// 这里为了减少重复代码，转型是适当的：
class TextBlock1 {
public:
    const char& operator[](std::size_t position) const {
        // 假设这里有非常多的代码
        return text[position];
    }
    char& operator[](std::size_t position) {
        return const_cast<char&>(static_cast<const TextBlock1&>(*this)[position]);
    }
private:
    std::string text;
};
// 注意，反向做法：令const版本调用non-const版本以避免重复并不被建议，一般而言
// const版本的限制比non-const版本的限制更多，会带来风险。

/*
item4： 确定对象在使用前已被初始化
无初值对象在C/C++中广泛存在，因此这一条款就尤为重要。
在定义完一个对象后需要尽快为它赋初值。
*/
int x = 0;
const char* text = "A C-style string";
// 对于类中的成员变量，有两种建议的方法完成初始化工作
// 1. 直接在定义处赋初值(C++11)
class CTextBlock1 {
private:
    std::size_t textLength{0};
    bool lengthValid = false;
};
// 2. 使用构造函数成员初始化列表
class ABEntry {
public:
    ABEntry(const std::string& name, const std::string& address) :
            theName(name), theAddress(address), numTimesConsulted(0) {

    }
    // 成员初始化列表也可以留空来执行默认构造函数
    ABEntry() : theName(), theAddress(), numTimesConsulted(0) {

    }
private:
    std::string theName;
    std::string theAddress;
    int numTimesConsulted;
};
// 注意：类中成员的初始化具有次序性，而这次序与成员变量的声明次序一致，
// 与成员初始化列表的次序无关
// 类中成员的初始化是可选的，但是引用类型必须初始化

// 静态对象的初始化
// C++对于定义于不同编译单元内的全局静态对象的初始化相对次序并没有明确定义
// 因此，以下代码可能会出现使用未初始化静态对象的情况
// File 1
class FileSystem {};
extern FileSystem tfs;
// File 2
class Directory {
public:
    Directory() {
        FileSystem disk = tfs;
    }
};
Directory tempDir;
// 上面示例，无法确保位于不同编译单元内的tfs一定在tempDir之前初始化完成
// 一个有效解决方案是采用Meyers’ singleton，将全局静态对象转化为局部静态对象
FileSystem& createTfs() {
    static FileSystem fs;
    return fs;
}
Directory& createTempDir() {
    static Directory td;
    return td;
}
// 这个手法的基础在于：C++保证，函数内的局部静态对象会在该函数被调用期间和
// 首次遇上该对象之定义式时被初始化。
// 当然，这种做法对于多线程来说并不具有优势，最好还是在单线程启动阶段手动调用函数完成初始化。

int main() {
    std::cout << "hello\n";

    return 0;
}