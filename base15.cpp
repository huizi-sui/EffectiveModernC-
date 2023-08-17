#include <iostream>

class Airplane {
private:
    struct AirplaneRep {
        unsigned long miles{10}; // 8
        char type{'A'}; // 1    16(字节对齐)
    };
    union {
        AirplaneRep rep{}; // 16
        Airplane* next; // 8
    };
public:
    unsigned long getMiles() {
        return rep.miles;
    }
    char getType() {
        return rep.type;
    }
    void set(unsigned long m, char t) {
        rep.miles = m;
        rep.type = t;
    }
    static void* operator new(size_t size);
    static void operator delete(void* ptr);
    ~Airplane() {
        std::cout << "Airplane::~Airplane()" << std::endl;
    }
    /// @brief 声明
    static const int BLOCK_SIZE;
    static Airplane *headOffFreeList;
};

/// 定义和初始化(c++11中类内部静态成员变量进行初始化是支持的，因此可以不写下面两行，但需要在类内初始化)
/// 位于静态区
Airplane* Airplane::headOffFreeList;
const int Airplane::BLOCK_SIZE = 512;


void* Airplane::operator new(size_t size) {
    Airplane* p = headOffFreeList;
    if(p) {
        headOffFreeList = p->next;
    } else {// ::operator new() 调用的是全局的，不是类内的
        Airplane* newBlock = (Airplane*)(::operator new(BLOCK_SIZE * sizeof(Airplane)));
        for(int i = 1; i < BLOCK_SIZE - 1; ++i) {
            newBlock[i].next = &newBlock[i + 1];
        }
        newBlock[BLOCK_SIZE - 1].next = nullptr;
        p = newBlock;
        headOffFreeList = &newBlock[1];
    }
    return p;
}

void Airplane::operator delete(void* ptr) {
    std::cout << "operator delete(void* ptr)" << std::endl;
    if(ptr == nullptr) {
        return;
    }
    Airplane* deleteMe = static_cast<Airplane*>(ptr);
    deleteMe->next = headOffFreeList;
    headOffFreeList = deleteMe;
}

int main() {
    std::cout << Airplane::headOffFreeList << std::endl;
    Airplane* p3 = new Airplane();
    size_t size = sizeof(Airplane);
    std::cout << size << std::endl;
    std::cout << p3 << std::endl;

    Airplane* p4 = new Airplane();
    std::cout << p4 << std::endl;

    Airplane* p5 = new Airplane();
    std::cout << p5 << std::endl;

    delete p3;
    delete p4;
    delete p5;

    return 0;
}