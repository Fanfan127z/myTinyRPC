#include<iostream>
#include <stdio.h>
#include <sys/time.h>
#include <sstream>
#include <time.h>
using namespace std;
// 用于格式化字符串。它使用了可变参数模板，可以接受任意数量和类型的参数。函数首先使用snprintf函数计算格式化后的字符串长度，然后创建一个字符串对象，并调整其大小以适应格式化后的字符串。最后，
// 使用snprintf函数将格式化后的字符串写入到字符串对象中，并返回该对象。
template<class... Args>
std::string formatString(const char* str, Args&& ... args){
    int size = snprintf(nullptr,0,str,args ...);
    std::string result;
    if(size > 0){
        result.resize(size);
        snprintf(&result[0], size+1, str , args...);
    }
    return result;
}


void func();
int main(){
    func();
    // std::string name = "linzhuofan";
    // auto ret = formatString("Name is : [%s]",name.c_str());
    // std::string str2 = "test log %s\n";
    // auto ret2 = formatString("Name is : [%s]", "11"); 
    // cout << "ret = " << ret << endl;
    // cout << "ret2 = " << ret2 << endl;
    // int age = 25;
    // std::string name2 = "John";
    // double height = 1.75;
    // std::string result = formatString("My name is %s, I am %d years old, and my height is %.2f meters.", name2.c_str(), age, height);
    // cout << "result = " << result << endl;

    // struct timeval tv;
    // gettimeofday(&tv, NULL);
    // printf("Seconds since epoch: %ld\n", tv.tv_sec);
    // printf("Microseconds: %ld\n", tv.tv_usec);
    
    // struct tm tm_info;
    // localtime_r(&tv.tv_sec,&tm_info);
    // printf("Current date and time: %s", asctime(&tm_info));
    // printf("Current year: %d\n", (&tm_info)->tm_year + 1900);
    // printf("Current month: %d\n", (&tm_info)->tm_mon + 1);
    // printf("Current day: %d\n", (&tm_info)->tm_mday);
    // printf("Current hour: %d\n", (&tm_info)->tm_hour);
    // printf("Current minute: %d\n", (&tm_info)->tm_min);
    // printf("Current second: %d\n", (&tm_info)->tm_sec);
    // printf("Current millisecond: %ld\n", tv.tv_usec / 1000);
    

    // time_t now = time(NULL);
    // struct tm *t = localtime(&now);
    // char buffer[80];
    // cout << "sizeof(buffer) = "<<sizeof(buffer)<<endl;
    // strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", t);
    // printf("当前时间为：%s\n", buffer);


    //     stringstream ss;
    // int a = 123;
    // double b = 3.14;
    // string c = "hello";

    // // 将不同类型的数据插入到stringstream中
    // ss << a << " " << b << " " << c;

    // // 从stringstream中读取数据
    // int a1;
    // double b1;
    // string c1;
    // ss >> a1 >> b1 >> c1;

    // // 输出读取到的数据
    // cout << a1 << " " << b1 << " " << c1 << endl;
    // cout << "ss.str() = " << ss.str() << endl;
    return 0;
}