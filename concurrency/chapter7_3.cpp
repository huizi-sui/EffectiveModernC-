#include <iostream>
#include <thread>
#include <atomic>
/*
C++11标准库中的std::atomic针对整型和指针类型的特化版本新增了一些
算术运算和逻辑运算操作。
*/

// fetch_add
/*
If T is integral
T fetch_add(T val, memory_order sync = memory_order_seq_cst) volatile noexcept;
T fetch_add(T val, memory_order sync = memory_order_seq_cst) noexcept;
If T is pointer
T fetch_add(ptrdiff_t val, memory_order sync = memory_order_seq_cst) volatile noexcept;
T fetch_add(ptrdiff_t val, memory_order sync = memory_order_seq_cst) noexcept;
将原子对象的封装值加val，并返回原子对象的旧值，整个过程是原子的。
sync可取Relaxed、Comsume、Acquire、Release、Acquire/Release、Sequentially consistent

若sync不指定，默认参数memory_order_seq_cst，则fetch_add相当于std::atomic::operator+=
*/

// fetch_sub
/*
If T is integral
T fetch_sub(T val, memory_order sync = memory_order_seq_cst) volatile noexcept;
T fetch_sub(T val, memory_order sync = memory_order_seq_cst) noexcept;
If T is pointer
T fetch_sub(ptrdiff_t val, memory_order sync = memory_order_seq_cst) volatile noexcept;
T fetch_sub(ptrdiff_t val, memory_order sync = memory_order_seq_cst) noexcept;
将原子对象的封装值减val，并返回原子对象的旧值，整个过程是原子的。
sync可取Relaxed、Comsume、Acquire、Release、Acquire/Release、Sequentially consistent

若sync不指定，默认参数memory_order_seq_cst，则fetch_add相当于std::atomic::operator-=
*/

// fetch_and
/*
If T is integral
T fetch_and(T val, memory_order sync = memory_order_seq_cst) volatile noexcept;
T fetch_and(T val, memory_order sync = memory_order_seq_cst) noexcept;

不适用于指针的特化版本

将原子对象的封装值按位与val，并返回原子对象的旧值，整个过程是原子的。
sync可取Relaxed、Comsume、Acquire、Release、Acquire/Release、Sequentially consistent

若sync不指定，默认参数memory_order_seq_cst，则fetch_add相当于std::atomic::operator&=
*/

// fetch_or
/*
If T is integral
T fetch_or(T val, memory_order sync = memory_order_seq_cst) volatile noexcept;
T fetch_or(T val, memory_order sync = memory_order_seq_cst) noexcept;

不适用于指针的特化版本

将原子对象的封装值按位或val，并返回原子对象的旧值，整个过程是原子的。
sync可取Relaxed、Comsume、Acquire、Release、Acquire/Release、Sequentially consistent

若sync不指定，默认参数memory_order_seq_cst，则fetch_add相当于std::atomic::operator|=
*/

// fetch_xor
/*
If T is integral
T fetch_xor(T val, memory_order sync = memory_order_seq_cst) volatile noexcept;
T fetch_xor(T val, memory_order sync = memory_order_seq_cst) noexcept;

不适用于指针的特化版本

将原子对象的封装值按位异或val，并返回原子对象的旧值，整个过程是原子的。
sync可取Relaxed、Comsume、Acquire、Release、Acquire/Release、Sequentially consistent

若sync不指定，默认参数memory_order_seq_cst，则fetch_add相当于std::atomic::operator^=
*/

// operator++
/*
pre-increment 前缀++
T operator++() volatile noexcept;
T operator++()  noexcept;
post-increment 后缀++(使用一个虚拟参数(int)来表示是后缀版本)
T operator++(int) volatile  noexcept;
T operator++(int)  noexcept;
适用于整型和指针类型的std::atomic特化版本
*/

// operator--
/*
pre-increment 前缀--
T operator--() volatile noexcept;
T operator--()  noexcept;
post-increment 后缀--(使用一个虚拟参数(int)来表示是后缀版本)
T operator--(int) volatile noexcept;
T operator--(int)  noexcept;
适用于整型和指针类型的std::atomic特化版本
*/