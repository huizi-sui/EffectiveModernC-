/*
item16: 让const成员函数线程安全

请记住
1. 确保const成员函数线程安全，除非你确定它们永远不会在并发上下文（concurrent context）中使用。
2. 使用std::atomic变量可能比互斥量提供更好的性能，但是它只适合操作单个变量或内存位置。
*/

#include <iostream>
#include <vector>
#include <mutex>
#include <atomic>
#include <math.h>

// 用一个类表示多项式
class Polynomial {
public:
    using RootsType = std::vector<double>;

    RootsType roots() const {
        // 计算多项式的零点
        // 不应该更改多项式，因此设置为const函数
        // 从概念上讲，roots并不改变它所操作的Polynomial对象，但是作为缓存的一部分
        // 它也许会改变rootVals和rootsAreValid的值。所以使用mutable
        // 计算多项式的根是复杂的，如果不需要就不做，如果做，应该缓存下来，这样就不用再做第二次
        if(!rootsAreValid) {
            // 计算根, 并存入rootVals
            rootsAreValid = true;
        }
        return rootVals;
    }

private:
    mutable bool rootsAreValid{false};
    mutable RootsType rootVals{};
};

// 解决线程安全最普遍简单的做法就是使用mutex（互斥量）
class Polynomial1 {
public:
    using RootsType = std::vector<double>;

    RootsType roots() const {
        std::lock_guard<std::mutex> g(m); // 锁定互斥量
        if(!rootsAreValid) { // 缓存无效
            // 计算根, 并存入rootVals
            rootsAreValid = true;
        }
        return rootVals;
    } // 解锁互斥量

private:
    // std::mutex m被声明为mutex，因为锁定和解锁它都是non-const成员函数
    // 在roots中，m却被视为const对象
    // std::mutex既不可以移动，也不可以复制。因而包含他们的类也同时是不可以移动和复制的。
    mutable std::mutex m;
    mutable bool rootsAreValid{false};
    mutable RootsType rootVals{};
};

// 某些情况下，互斥量的副作用会显得过大。例如计算成员函数被调用了多少次
// 使用std::atomic修饰的计算器通常会是一个开销更小的方法
class Point {
public:
    double distanceFromOrigin() const noexcept {
        ++callCount;
        return std::sqrt((x * x ) + (y * y));
    }
private:
    // 与 std::mutex 类似的，实际上 std::atomic 既不可移动，也不可复制。
    // 因而包含他们的类也同时是不可移动和不可复制的。
    mutable std::atomic<unsigned> callCount{0};
    double x, y;
};

// 因为对std::atomic变量的操作通常比互斥量的获取和释放的消耗更小，
// 所以你可能会过度倾向与依赖std::atomic。
// 例如，缓存一个开销昂贵的int，你就会尝试使用一对std::atomic变量而不是互斥量。
// 这是可行的。但难以避免有时出现重复计算的情况。考虑：
// 1. 一个线程调用Widget::magicValue, 将cacheVild视为false，执行这两个昂贵的计算，
// 并将它们的分配给cachedValue
// 2. 此时，第二个线程调用Widget::magicValue，也将cacheVaild视为false，因此执行
// 刚才完成的第一个线程相同的计算。（这里第二个线程实际上可能是其他几个线程）。
class Widget {
public:
    int magicValue() const {
        if(cacheVaild) {
            return cacheValue;
        } else {
            auto val1 = expensiveCompulation1();
            auto val2 = expensiveCompulation2();
            cacheValue = val1 + val2; // 第一步
            cacheVaild = true; // 第二步
            return cacheValue;
        }
    }
    int expensiveCompulation1() const {
        int res = 2;
        // ... 进行昂贵的计算
        return res;
    }
    int expensiveCompulation2() const {
        int res = 3;
        // ...进行昂贵的计算
        return res;
    }
private:
    mutable std::atomic<bool> cacheVaild{false};
    mutable std::atomic<int> cacheValue;
};

// 交换第一步和第二步可以解决这个问题，但结果会更糟糕
// 假设cacheVaild是false，则
// 1. 一个线程调用Widget::magicValue，刚执行完将cacheVaild设置为true语句
// 2. 在这时，第二个线程调用Widget::magicValue，检查cacheVaild。看到它为true，
// 就返回cacheValue，即使第一个线程没有给它赋值。因此返回的值是不正确的。
class Widget1 {
public:
    int magicValue() const {
        if(cacheVaild) {
            return cacheValue;
        } else {
            auto val1 = expensiveCompulation1();
            auto val2 = expensiveCompulation2();
            cacheVaild = true; // 第一步
            return cacheValue = val1 + val2; // 第二步
        }
    }
    int expensiveCompulation1() const {
        int res = 2;
        // ... 进行昂贵的计算
        return res;
    }
    int expensiveCompulation2() const {
        int res = 3;
        // ...进行昂贵的计算
        return res;
    }
private:
    mutable std::atomic<bool> cacheVaild{false};
    mutable std::atomic<int> cacheValue;
};

// 所以，对于需要同步的是单个的变量或者内存位置，使用std::atomic就足够
// 一旦需要对两个以上的变量或者内存位置作为一个单元来操作，就应该使用互斥量
class Widget2 {
public:
    int magicValue() const {
        std::lock_guard<std::mutex> g(m); // 锁定m

        if(cacheVaild) {
            return cachedValue;
        } else {
            auto val1 = expensiveCompulation1();
            auto val2 = expensiveCompulation2();
            cachedValue = val1 + val2;
            cacheVaild = true;
            return cachedValue;
        }
    } // 解锁m
    int expensiveCompulation1() const {
        int res = 2;
        // ... 进行昂贵的计算
        return res;
    }
    int expensiveCompulation2() const {
        int res = 3;
        // ...进行昂贵的计算
        return res;
    }
private:
    mutable std::mutex m;
    mutable int cachedValue;
    mutable bool cacheVaild{false};
};

int main() {
    {
        Polynomial p;
        // 假设有两个线程，没有做到线程安全。
        // 这些代码会有不同的线程读写相同的内存，这就是数据竞争（data race）的定义
        // 这段代码的行为是未定义的
        // 线程1
        auto rootsOfp = p.roots();
        // 线程2
        auto valGivingZero = p.roots();
    }
}