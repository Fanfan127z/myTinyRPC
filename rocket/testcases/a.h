#ifndef A_H
#define A_H
#include <iostream>
class A{
public:
    A() { printf("A construct called.\n"); };
    static A* func_get_global_A();
    ~A() { printf("A deconstruct called.\n"); };
};
#endif // A_H