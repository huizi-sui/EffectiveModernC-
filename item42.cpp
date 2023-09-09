/*
item42: 考虑就地创建而非插入

使得置入（emplacement）函数功能优于插入函数的原因是它们有灵活的接口。
插入函数接受对象去插入，而置入函数接受对象的构造函数接受的实参去插入。
这种差异允许置入函数避免插入函数所必需的临时对象的创建和销毁。
请记住：

1. 原则上，置入函数有时会比插入函数高效，并且不会更差。
1. 实际上，当以下条件满足时，置入函数更快：
（1）值被构造到容器中，而不是直接赋值；
（2）传入的类型与容器的元素类型不一致；
（3）容器不拒绝已经存在的重复值。
2. 置入函数可能执行插入函数拒绝的类型转换。
*/
#include <iostream>
#include <vector>
#include <list>
#include <regex>
#include <memory>

void test1() {
    std::vector<std::string> vs; 
    vs.push_back("xyzzy"); // 添加字符串字面量
    /*
    字符串字面量不是std::string，这意味着传递给push_back的实参
    不是容器里的内容类型。
    编译器看到实参类似(const char[6])和push_back采用的形参类型(std::string的引用)不匹配。
    通过字符串字面量创建一个std::string类型的临时对象来消除不匹配，然后传递临时变量给push_back。
    所以编译器处理这个调如: vs.push_back(std::string("xyzzy"));
    但是这里调用了两次构造函数，而且还调用了一次析构函数。
    1. 一个std::string的临时对象从字面量"xyzzy"被创建。这个对象没有名字，可以称为temp。
    temp的构造是第一次std::string构造。因为是临时变量，所以temp是右值。
    2. temp被传递给push_back的右值重载函数，绑定到右值引用形参x。在std::vector的内存中一个x的副本被创建。
    这次构造(第二次构造)——在std::vector内部真正创建一个对象。(将x副本拷贝到std::vector内部的构造函数是
    移动构造函数，因为x在它被拷贝之前被转换为右值，成为右值引用。)
    3. 在push_back返回之后，temp立刻被销毁，调用一次std::string的析构函数。
    */
    vs.emplace_back("xyzzy");
    /*
    emplace_back: 使用传递给它的任何实参直接在std::vector内部构造一个std::string。没有临时对象产生。
    emplace_back使用完美转发，因此只要没有遇到完美转发的限制(item30)，就可以传递任何实参以及组合到emplace_back。
    */
    vs.emplace_back(50, 'x'); // 插入由50个“x”组成的一个std::string
    /*
    emplace_back ====   push_back
    emplace_front ===   emplace_front
    insert(除了std::forward_list和std::array) ===   emplace
    emplace_hint ===    hint迭代器的insert
    empalce_after(std::forward_list) ===    insert_after
    */
}
// std::vector的push_back被按左值和右值分别重载，类似如下
// C++11
template<class T, class Allocator = std::allocator<T>>
class vector1 {
public:
    void push_back(const T& x); // 插入左值
    void push_back(T&& x); // 插入右值
};

/*
因为可以传递容器内元素类型的实参给置入函数，所以在插入函数不会构造临时对象的情况，
也可以使用置入函数。在这种情况下，插入和置入函数做的是同一件事情
*/
void test2() {
    std::string queenOfDisco("Donna Summer");
    std::vector<std::string> vs;
    vs.push_back(queenOfDisco); // 拷贝构造queenOfDisco
    vs.emplace_back(queenOfDisco); // 拷贝构造queenOfDisco
    // 因此，置入函数可以完成插入函数的所有功能。
    // 并且有时候效率更高，至少在理论上，不会更低效。
    // 为什么不再所有场合上使用它们？
}
/*
但只是理论上。有些场景，置入执行性能优于插入，但是有些场景插入更快。
大致的调用建议是：通过benchmark测试来确定置入和插入哪种更快。
*/
/*
如果下面条件满足，置入会优先于插入：
1. 值是通过构造函数添加到容器，而不是直接赋值。
示例如test1()函数所示。
用“xyzzy”添加std::string到std::vector容器vs中，值添加到vs的末尾——
一个先前没有对象存在的地方。新值必须通过构造函数添加到std::vector中。
新值放到已经存在了对象的一个地方，情况就不一样了。
2. 传递的实参类型与容器的初始化类型不同。
再次强调，置入优于插入通常基于以下事实：当传递的实参不是容器保存的类型时，
接口不需要创建和销毁临时对象。当将类型为T的对象添加到container<T>时，
没有理由期望置入比插入运行的更快，因为不需要创建临时对象来满足插入的接口。
3. 容器不拒绝重复项作为新值。 
这意味着容器要么允许添加重复值，要么你添加的元素大部分都是不重复的。
这样要求的原因是为了判断一个元素是否已经存在于容器中，
置入实现通常会创建一个具有新值的节点，以便可以将该节点的值与现有容器中节点的值进行比较。
如果要添加的值不在容器中，则链接该节点。然后，如果值已经存在，置入操作取消，创建的节点被销毁，
意味着构造和析构时的开销被浪费了。这样的节点更多的是为置入函数而创建，相比起为插入函数来说。

vs.emplace_back("xyzzy"); //在容器末尾构造新值；不是传递的容器中元素的类型；没有使用拒绝重复项的容器
vs.emplace_back(50, 'x'); 
*/
void test2() {
    std::vector<std::string> vs;
    std::string s("aaa");
    vs.push_back(s);
    vs.push_back(s);
    vs.push_back(s);

    vs.emplace(vs.begin(), "xyzzy"); // 添加到vs头部
    /*
    没有实现会在已经存在对象的位置vs[0]构造这个添加的std::string。
    而是，通过移动赋值的方式添加到需要的位置。
    但是移动赋值需要一个源对象，所以这意味着一个临时对象要被创建，
    而置入优于插入的原因就是没有临时对象的创建和销毁，
    所以当通过赋值操作添加元素时，置入的优势消失殆尽。
    而且，向容器添加元素是通过构造还是赋值通常取决于实现者。
    */
}
class Widget {

};
/*
决定是否使用置入函数时，需要注意另外两个问题。
1. 资源管理
应该使用std::make_shared来创建std::shared_ptr，
但是如果指定一个自定义删除器时，它无法做到，必须直接new一个原始指针，
然后通过std::shared_ptr来管理
*/
std::list<std::shared_ptr<Widget>> ptrs;
// 自定义的删除器
void killWidget(Widget* pWidget) {
    delete pWidget;
}
void test3() {
    ptrs.push_back(std::shared_ptr<Widget>(new Widget, killWidget));
    // 也可以这样写
    ptrs.push_back({new Widget, killWidget});
    // 不管哪种写法，在调用push_back前会生成一个临时std::shared_ptr对象。
    // push_back的形参是std::shared_ptr的引用，因此必须有一个std::shared_ptr。
    // 用emplace_back应该可以避免std::shared_ptr临时对象的创建，但是在这个场景下，临时对象值得被创建。
    /*
    考虑如下可能的时间序列：
    1. 在上述的调用中，一个std::shared_ptr<Widget>的临时对象被创建来持有“new Widget”返回的原始指针。称这个对象为temp。
    2. push_back通过引用接受temp。在存储temp的副本的list节点的内存分配过程中，内存溢出异常被抛出。
    3，随着异常从push_back的传播，temp被销毁。作为唯一管理这个Widget的std::shared_ptr，
    它自动销毁Widget，在这里就是调用killWidget。
    这样，即使发生了异常，没有资源泄露：在调用push_back中通过“new Widget”创建的Widget在
    std::shared_ptr管理下自动销毁，生命周期良好。
    */
    ptrs.emplace_back(new Widget, killWidget);
    /*
    1. 通过new Widget创建的原始指针完美转发给emplace_back中，list节点被分配的位置。如果分配失败，
    还是抛出内存溢出异常。
    2. 当异常从emplace_back传播，原始指针是仅有的访问堆上Widget的途径，但是因为异常而丢失了，那个
    Widget的资源（以及任何它所拥有的资源）发生了泄露。
    */
    // 消除资源泄漏可能性的方法是，使用独立语句把从“new Widget”获取的指针传递给资源管理类对象，
    // 然后这个对象作为右值传递给你本来想传递“new Widget”的函数
    std::shared_ptr<Widget> spw(new Widget, killWidget);
    ptrs.push_back(std::move(spw));
    // empalce_back版本
    std::shared_ptr<Widget> spw1(new Widget, killWidget);
    ptrs.emplace_back(std::move(spw1));
    // 无论哪种方式，都会产生spw的创建和销毁成本
    // 选择置入而非插入的动机是避免容器元素类型的临时对象的开销。
    // 但是对于spw的概念来讲，当添加资源管理类型对象到容器中，
    // 并根据正确的方式确保在获取资源和连接到资源管理对象上之间无其他操作时，置入函数不太可能胜过插入函数。
}
/*
置入函数的第二个值得注意的方面是：
它们与explicit的构造函数的交互。
鉴于C++11对正则表达式的支持，假设你创建了一个正则表达式对象的容器
*/
std::vector<std::regex> regexes;
void test4() {
    // 指针不是正则表达式
    // std::regex r = nullptr; // error: 不能编译
    // regexes.push_back(nullptr); // error: 不能编译
    regexes.emplace_back(nullptr); // 可以编译
    std::regex upperCaseWorld("[A-Z]+");
    // 通过字符串创建std::regex要求相对较长的运行时开销，
    // 所以为了最小程度减少无意中产生此类开销的可能性，
    // 采用const char*指针的std::regex构造函数是explicit的。
    // 这就是上面nullptr不能编译的原因。
    // 上面要求从指针到std::regex的隐式转换，但是构造函数的explicit拒绝此类转换。
    // 在emplace_back调用中，我们没有说明要传递一个std::regex对象。然而，我们传递了一个
    // std::regex构造函数实参。那不被认为是个隐式转换要求。
    // 相反，编译器看你像是写了如下代码
    std::regex r(nullptr); // 可编译，但是行为不确定
    // 使用const char*指针的std::regex构造函数要求字符串是一个有效的正则表达式，空指针并不满足要求。
    // std::regex r1 = nullptr; // 错误。不能编译
    // std::regex r2(nullptr); // 可以编译
    // 用于初始化r1的语法是所谓的拷贝初始化，用于初始化r2的语法被称为直接初始化。
    /*
    using regex = basic_regex<char>;
    explicit basic_regex(const char* ptr, flag_type flags); // 定义1， explicit构造函数
    basic_regex(const basic_regex& right); // 定义2，拷贝构造函数
    拷贝初始化不被允许使用explicit构造函数（译者注：即没法调用相应类的explicit拷贝构造函数）：
    对于r1,使用赋值运算符定义变量时将调用拷贝构造函数定义2，其形参类型为basic_regex&。
    因此nullptr首先需要隐式装换为basic_regex。而根据定义1中的explicit，这样的隐式转换不被允许，从而产生编译时期的报错。
    对于直接初始化，编译器会自动选择与提供的参数最匹配的构造函数，即定义1。就是初始化r1不能编译，而初始化r2可以编译的原因。

    然后回到push_back和emplace_back，更一般来说是，插入函数和置入函数的对比。
    置入函数使用直接初始化，这意味着可能使用explicit的构造函数。
    插入函数使用拷贝初始化，所以不能用explicit的构造函数.
    */
}


int main() {

}