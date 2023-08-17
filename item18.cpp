#include <iostream>
#include <memory>


class Test {
public:
    Test() {
        std::cout << "Test()" << std::endl;
    }
    ~Test() {
        std::cout << "~Test()" << std::endl;
    }
};

class Investment {
public:
    virtual ~Investment() {

    }
};

class Stock : public Investment {
public:
    Stock(int a) {
        std::cout << "Stock(int a)" << std::endl;
    }
    ~Stock() override {
        std::cout << "~Stock()" << std::endl;
    }
};

class Bond : public Investment {
public:
    Bond(int a, int b) {
        std::cout << "Bond(int a, int b)" << std::endl;
    }
    ~Bond() override {
        std::cout << "~Bond()" << std::endl;
    }
};

class RealEstate : public Investment {
public:
    RealEstate(int a, int b, int c) {
        std::cout << "RealEstate(int a, int b, int c)" << std::endl;
    }
    ~RealEstate() override {
        std::cout << "~RealEstate()" << std::endl;
    }
};

template<typename... Ts>
Investment* makeInverstment_test(Ts&& ...params) {
    Investment* ptr;
    constexpr int numArgs = sizeof...(params);
    if constexpr (numArgs == 1) {
        Stock stock(std::forward<Ts>(params)...);
        ptr = &stock;
    }
    if constexpr (numArgs == 2) {
        Bond bond(std::forward<Ts>(params)...);
        ptr = &bond;
    }
    if constexpr (numArgs == 3) {
        RealEstate realEstate(std::forward<Ts>(params)...);
        ptr = &realEstate;
    }
    return ptr;
}

template<typename... Ts>
Investment* makeInverstment_test2(Ts&& ...params) {
    Investment* ptr;
    constexpr int numArgs = sizeof...(params);
    if constexpr (numArgs == 1) {
        Stock* stock = new Stock(std::forward<Ts>(params)...);
        ptr = stock;
    }
    if constexpr (numArgs == 2) {
        Bond* bond = new Bond(std::forward<Ts>(params)...);
        ptr = bond;
    }
    if constexpr (numArgs == 3) {
        RealEstate* realEstate = new RealEstate(std::forward<Ts>(params)...);
        ptr = &realEstate;
    }
    return ptr;
}

template<typename... Ts>
std::unique_ptr<Investment> makeInverstment(Ts&& ...params) {
    std::unique_ptr<Investment> uptr(nullptr);
    constexpr int numArgs = sizeof...(params);
    if constexpr (numArgs == 1) {
        uptr.reset(new Stock(std::forward<Ts>(params)...));
    }
    if constexpr (numArgs == 2) {
        uptr.reset(new Bond(std::forward<Ts>(params)...));
    }
    if constexpr (numArgs == 3) {
        uptr.reset(new RealEstate(std::forward<Ts>(params)...));
    }
    return uptr;
}

// 自定义删除器
auto delInvmt = [](Investment* pInvestment) {
    std::cout << "delete" << std::endl;
    delete pInvestment;
};

// C++14中auto可以用作函数返回值类型，这时自定义删除器可以定义在模板函数内部

template<typename... Ts>
std::unique_ptr<Investment, decltype(delInvmt)> makeInverstment2(Ts&& ...params) {
    std::unique_ptr<Investment, decltype(delInvmt)> uptr(nullptr, delInvmt);
        constexpr int numArgs = sizeof...(params);
    if constexpr (numArgs == 1) {
        uptr.reset(new Stock(std::forward<Ts>(params)...));
    }
    if constexpr (numArgs == 2) {
        uptr.reset(new Bond(std::forward<Ts>(params)...));
    }
    if constexpr (numArgs == 3) {
        uptr.reset(new RealEstate(std::forward<Ts>(params)...));
    }
    return uptr;
}

int main() {
    {
        // unique_ptr只允许移动，不允许拷贝
        // 移动之后，源指针为nullptr
        std::unique_ptr<Test> uptr = std::unique_ptr<Test>(new Test());
        std::cout << uptr.get() << std::endl;
        std::unique_ptr<Test> uptr1 = std::move(uptr);
        std::cout << uptr.get() << std::endl;
        std::shared_ptr<Test> uptr2 = std::move(uptr1);
    }
    {
        // 但是如果匿名函数具有很多状态，则产生的unique_ptr过大，需要修改
        std::unique_ptr<Investment, void(*)(Investment*)> uptr1(nullptr, delInvmt); // 16
        std::unique_ptr<Investment, decltype(delInvmt)> uptr2(nullptr, delInvmt); // 8
        std::cout << sizeof(uptr1) << "   " << sizeof(uptr2) << std::endl;
    }
}