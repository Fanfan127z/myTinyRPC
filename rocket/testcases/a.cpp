#include "a.h"

static A* g_a= nullptr;
A* A::func_get_global_A(){
    if(!g_a)return g_a;
    g_a = new A();
    return g_a;
}