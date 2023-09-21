#include <iostream>
#include <memory>

/*
item13: 以对象管理资源

对于传统的堆资源管理，需要使用成对的new和delete，
这样若忘记delete就会造成内存泄露。

因此，应尽可能使用对象管理资源，并采用RAII(Resource Acquisition 
Is Initialize, 资源取得时机便是初始化时机)，让析构函数负责资源的释放。

原书内容已过时，C++11中，通过专一所有权来管理RAII对象可以使用std::unique_ptr,
通过引用计数来管理RAII对象可以使用std::shared_ptr。
*/
class Inverstment {

};
Inverstment* CreateInverstment() {
    Inverstment* inv = new Inverstment;
    return inv;
}
void test1() {
    std::unique_ptr<Inverstment> pUniqueInv1(CreateInverstment());
    std::unique_ptr<Inverstment> pUniqueInv2(std::move(pUniqueInv1)); // 转移资源所有权

    std::shared_ptr<Inverstment> pSharedInv1(CreateInverstment());
    std::shared_ptr<Inverstment> pSharedInv2(pSharedInv1); // 引用计数+1
}
// 智能指针默认会自动delete所持有的对象，也可以为智能指针指定所管理对象的释放方式(删除器deleter)
void GetRidOfInvestment(Inverstment* inv) {
    std::cout << "Inverstment deleter" << std::endl;
    delete inv;
}
void test2() {
    /*
    对于std::unique_ptr来说，删除器类型是智能指针类型的一部分
    对于std::shared_ptr来说则不是。
    且指定自定义删除器不会改变std::shared_ptr对象的大小
    不管删除器是什么，一个std::shared_ptr对象都市两个指针大小
    */
    std::unique_ptr<Inverstment,  void(*)(Inverstment*)> pUniqueInv(CreateInverstment(), GetRidOfInvestment);
    std::shared_ptr<Inverstment> pSharedInv(CreateInverstment(), GetRidOfInvestment);
}

/*
item14：在资源管理类中小心拷贝行为

应当思考： 当一个RAII对象被复制会发生什么事情？

选择1：禁止复制
许多时候允许RAII对象复制并不合理，如果确实如此，则就该明确禁止复制行为

选择2：对底层资源祭出“引用计数法”
正如std::shared_ptr所做的那样，每一次复制对象就使引用计数+1，每一个对象
离开定义域就调用析构函数使引用计数-1，直到引用计数为0就彻底销毁资源。

选择3：复制底层资源
在复制对象的同时复制底层资源的行为又称为深拷贝，例如在一个对象中有一个指针，
那么在复制这个对象时就不能只复制指针，也要复制指针所指向的数据。

选择4：转移底层资源所有权
和std::unique_ptr的行为类似，永远保持只有一个对象拥有对资源的管理权，
当需要复制对象时转移资源的管理权。
*/

/*
item15：在资源管理类中提供对原始资源的访问
和所有的智能指针一样，STL中的智能指针也提供了对原始资源的隐式访问和显式访问
*/
void test3() {
    std::shared_ptr<Inverstment> pSharedInv(CreateInverstment());

    Inverstment* pRaw = pSharedInv.get(); // 显式访问原始资源
    Inverstment raw = *pSharedInv; // 隐式访问原始资源
}
// 当我们在设计自己的资源管理类时，也要考虑在提供对原始资源的访问时，
// 是使用显式访问还是隐式访问的方法，还是两者皆可。
class FontHandle {

};
class Font {
public:
    FontHandle get() const {
        return handle; // 显式转换函数
    }
    operator FontHandle() const {
        return handle; // 隐式转换函数
    }
private:
    FontHandle handle;
};
// 一般而言，显式转换比较安全，但隐式转换对客户比较方便

/*
item16：成对使用new和delete时要采用相同形式
使用new来分配单一对象，使用new[]来分配对象数组，必须明确他们的行为
并不一致，分配对象数组时会额外在内存中记录“数组大小”，而使用delete[]
会根据记录的数组大小多次调用析构函数，使用delete则仅仅只调用一次析构函数。
对于单一对象使用delete[]其结果也是未定义的，程序可能会读取若干内存并将其
错误地解释为数组大小。
*/
void test4() {
    int* array = new int[10];
    int* object = new int;
    delete[] array;
    delete object;

    // 需要注意的是，使用typedef定义数组类型会带来额外的风险
    // 在C++中，typedef关键字用于为类型创建别名。然而，在创建别名时，
    // 数组的大小必须是类型的一部分。因此，在类型别名的语法中，数组的大小
    // 应该在类型名称之后声明。
    typedef std::string AddressLines[4];
    std::string* pal = new AddressLines; // pal是一个对象数组，而非单一对象

    // delete pal; // 行为未定义
    delete[] pal;
}

/*
item17： 以独立语句将newed对象置入智能指针
原书已过时，现在更好的做法是使用std::make_unique和std::make_shared
*/
void test5() {
    auto pUniqueInv = std::make_unique<Inverstment>(); // since C++14
    auto pSharedInv = std::make_shared<Inverstment>(); // since C++11
}

int main() {
    test2();
}