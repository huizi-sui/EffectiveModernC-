/*
item8: 优先考虑nullptr而不是0和NULL
字面值0是一个int不是指针，如果C++发现在当前上下文只能使用指针，
它会不情愿的把0解释为指针。一般来说，C++解析策略是把0看作int而不是指针
NULL也是如此。但是还可以给NULL一个除了int之外的整数类型（比如long）。

nullptr的优点是它不是整型。老实说它也不是一个指针类型，但是可以把它认为是所有
类型的指针。
nullptr的真正类型是std::nullptr_t,在一个完美的循环定义后，std::nullptr_t又被
定义为nullptr。nullptr可以隐式转换为指向任何内置类型的指针。
*/
#include <iostream>
#include <memory>
#include <mutex>

void f(int ) {
    std::cout << "f(int)" << std::endl;
}

void f(bool) {
    std::cout << "f(bool)" << std::endl;
}

void f(void*) {
    std::cout << "f(void*)" << std::endl;
}

class Widget {

};

int f1(std::shared_ptr<Widget> spw) {
    std::cout << "f1(std::shared_ptr<Widget>)" << std::endl;
    return 1;
}

double f2(std::unique_ptr<Widget> upw) {
    std::cout << "f2(std::unique_ptr<Widget>)" << std::endl;
    return 1.0;
}

bool f3(Widget* w) {
    std::cout << "f3(Widget*)" << std::endl;
    return true;
}

template<typename FuncType, typename MuxType, typename PtrType>
auto lockAddCall(FuncType func, 
                 MuxType &mutex, 
                 PtrType ptr) -> decltype(func(ptr)) {
    using MuxGuard = std::lock_guard<std::mutex>;
    MuxGuard g(MuxType);
    return func(ptr);
}

int main() {
    f(0); // f(int)
    // 如果NULL被定义为0L，则从long到int， long到bool
    // long 到void*都可以
    // f(NULL); // error
    f(nullptr); // f(void*)
    {
        // auto result = findRecord();
        // if(result == 0) { // 不知道findRecord()函数返回的到底是什么
        //    
        // }
        // if(result == nullptr) {} // findRecord()返回的必定是指针
    }
    {
        // f1, f2, f3只能被合适的已锁互斥量调用
        // 令人遗憾的是前两个没有使用nullptr，但是代码正常运行
        // 重复调用代码：为互斥量上锁，调用函数，解锁互斥量
        std::mutex f1m, f2m, f3m; // 用于f1，f2，f3函数的互斥量
        using MuxGuard = std::lock_guard<std::mutex>;
        {
            MuxGuard g(f1m); // 为f1m上锁
            auto result = f1(0); // 向f1传递0作为空指针
            // 解锁
        }
        {
            MuxGuard g(f2m); // 为f2m上锁
            auto result = f2(NULL); // 向f2传递NULL作为空指针
            // 解锁
        }
        {
            MuxGuard g(f3m); // 为f3m上锁
            auto result = f3(nullptr); // 向f3传递nullptr作为空指针
            // 解锁
        }
    }
    {
        // 使用模板简化上述过程
        std::mutex f1m, f2m, f3m;
        /*
        0的类型是int
        传入locakAndCall中会被解析成int类型
        在lockAndCall中调用f1()，会与期待的std::shared_ptr<Widget>形参不符

        */
        // auto result1 = lockAddCall(f1, f1m, 0); // error
        /*
        与0类似，当NULL被传递给lockAndCall，形参ptr被推导为整型（int，long等类型），
        当int型传递给f2时出现类型错误，f2期待的是std::unique_ptr<Widget>
        */
        // auto result2 = lockAddCall(f2, f2m, NULL); // error
        /*
        当nullptr传递给lockAndCall时，ptr被推导为std::nullptr_t，
        当ptr传递给f3时，隐式转换使std::nullptr_t转化为Widget*,
        因为std::nullptr_t可以隐式转换为任何指针类型
        */
        auto result3 = lockAddCall(f3, f3m, nullptr);
    }
}