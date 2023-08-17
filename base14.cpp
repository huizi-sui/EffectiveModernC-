#include <iostream>
#include <vector>

// 完全掌握内存的申请和释放，重写类内的operator new/delete。
// 修改全局的operator new/delete不合理
class Test {
public:
    Test() {
        std::cout << "Test() addr is: " << this << std::endl;
    }
    ~Test() {
        std::cout << "~Test() addr is" << this << std::endl;
    }
    void* operator new(size_t size) {
        std::cout << "operator new(size_t size)" << std::endl;
        return malloc(size);
    }
    void operator delete(void* ptr, size_t size) { // 第二个参数不写也可
        std::cout << "operator delete(voi* ptr, size_t size)" << std::endl;
        free(ptr);
    }
    void* operator new(size_t size, void* buf) {
        std::cout << "operator new(size_t size, void* buf)" << std::endl;
        return buf;
    }
    void operator delete(void* ptr, void* buf) {
        std::cout << "operator delete(void* ptr, void* buf)" << std::endl;
        free(ptr);
    }
};

int main() {
    /*
        c语言的
        malloc单纯只是为了申请费一块固定大小的内存
        free单纯只是为了将内存释放掉
    */
    void* p1 = malloc(sizeof(Test) * 10);
    free(p1);

    /*
        C++， 做的事情比malloc/free多
        Test* p2 = new Test() 等价于  void* mem = operator new(sizeof(Test));
                                     Test* p2 = (Test*)mem;
                                     p2->Test();
        delete p2; 等价于   p2->~Test(); operator delete(p2);
    */
    Test* p2 = new Test();
    delete p2;

    // 第三种方式
    /*
        下面语句等价于
        void* buf = malloc(sizeof(Test));
        void* mem = operator new(sizeof(Test), buf);
        Test* p4 = (Test*)buf;
        p3->Test();
        称为placement new方法，该方法需要传入一块分配号内存的指针，其本身不会额外申请空间
        释放内存的时候必须delete p4，不能free p3。
        因为delete会先执行析构函数，再调用free，而free会错过析构函数的调用
    */
    void* buf = malloc(sizeof(Test));
    Test* p4 = new(buf)Test();
    delete p4;

    return 0;
}