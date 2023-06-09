// #include "../common/util.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <functional>
#include <algorithm>
#include <sstream>
#include <queue>
#include <thread>
#include <time.h>
#include <map> 
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


void func1(){
    printf("func1 called\n");
}
void func2(){
    printf("func2 called\n");
}
void func3(){
    printf("func3 called\n");
}
void print(std::queue<std::function<void ()>>& p){
    if(!p.empty()){
        printf("p != empty\n");
        while(!p.empty()){
            p.front()();p.pop();
        }
    }
}
const std::string timeStamp2RealTimeForMat(int64_t timestamp){
    // 因为timestamp是原始13位的时间戳，函数内部中间需要转换为10位的
    // time_t timestamp = 1627584000; // 2021-07-30 00:00:00
    // time_t timestamp = 1686209168636; // 1686209168636 ==> 标识 2023-06-08 15:26:08
    time_t t_timestamp = timestamp / 1000; // 标识真实时间的10位时间戳
    char str_time[100];
    memset(str_time, 0, sizeof(str_time));
    strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", localtime(&t_timestamp));
    // std::cout << "The time is: " << str_time << std::endl;
    return std::string(str_time);
}
__thread int g_a = 1;
void print_g_a(){printf("g_a=[%d]\n",g_a);}
void run1(){
    g_a++;
    print_g_a();
}
void run2(){
    ++g_a;
    print_g_a();
}
int main(){
    std::thread t1(run1);
    t1.join();
    std::thread t2(run2);
    t2.join();
    print_g_a();
    // std::multimap<int, int> m_pending_events;
    // m_pending_events.insert({1,123});
    // m_pending_events.insert({2,223});
    // m_pending_events.insert({3,323});
    // m_pending_events.insert({4,423});
    // m_pending_events.insert({4,4233});
    // m_pending_events.insert({5,523});
    // m_pending_events.insert({5,5235});
    // auto begin = m_pending_events.begin();
    // // for(auto it = m_pending_events.begin(); it != m_pending_events.end(); ++it){}
    // // printf("[(*begin).first:(*begin).second] == [%d:%d]\n", (*begin).first, (*begin).second);
    // // printf("[(*begin).first:(*begin).second] == [%d:%d]\n", (*begin).first, (*begin).second);
    // for(auto it = begin; it != m_pending_events.end(); ++it){}
    // printf("[(*begin).first:(*begin).second] == [%d:%d]\n", (*begin).first, (*begin).second);
    
    // time_t timestamp = 1686209168636; // 1627584000 ==> 标识2021-07-30 00:00:00
    // // std::cout << "The time is: " << timeStamp2RealTimeForMat(timestamp) << std::endl;// 
    // std::cout << "The time is: " << rocket::origin13bitTimeStamp2RealTimeForMat(timestamp) << std::endl;
    // time_t ts2 = timestamp / 1000; // 标识真实时间的10位时间戳
    // char str_time[100];
    // memset(str_time, 0, sizeof(str_time));
    // strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", localtime(&ts2));
    // std::cout << "The time is: " << str_time << std::endl;

    // long long timestamp = 1686209168636; // 原始时间戳
    // time_t t = (timestamp / 1000); // 将时间戳转换为秒数
    // struct tm *tm_now = localtime(&t); // 将秒数转换为本地时间
    // char buf[20];
    // strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_now); // 格式化时间字符串

    // std::cout << buf << std::endl; // 输出新的时间戳    


    // std::multimap<int, int> umap;
    // umap.insert({1,123});
    // umap.insert({2,223});
    // umap.insert({3,323});
    // umap.insert({4,423});
    // umap.insert({4,4233});
    // umap.insert({5,523});
    // umap.insert({5,5235});
    // for_each(umap.begin(), umap.end(),[](const pair<int, int>& p){
    //     printf("[%d, %d]\n", p.first, p.second);
    // });
    // // umap.erase(4);umap.erase(5);
    // auto begin = umap.begin();
    // auto it = umap.begin();
    // int cnt = 3;
    // while(it != umap.end() && (cnt--))it++;
    // printf("it = [%d, %d]\n", it->first, it->second);
    // umap.erase(begin, it);
    // printf("-------------------------\nafter umap.erase(begin, it):\n");
    // for_each(umap.begin(), umap.end(),[](const pair<int, int>& p){
    //     printf("[%d, %d]\n", p.first, p.second);
    // });
    // func();
    // std::queue<std::function<void ()>>m_pending_task;
    // m_pending_task.push(func1);
    // m_pending_task.push(func2);
    // m_pending_task.push(func3);

    // // printf("before swap:-----------\n");
    // // print(m_pending_task);
    // // m_pending_task.push(func1);
    // // m_pending_task.push(func2);
    // // m_pending_task.push(func3);
    // std::queue<std::function<void ()>> tmp_tasks;
    // m_pending_task.swap(tmp_tasks);
    // while(!m_pending_task.empty()){
    //     printf("m_pending_task != empty\n");
    //     m_pending_task.front()();m_pending_task.pop();
    // }
    // printf("after swap:-----------\n");
    // printf("tmp_tasks:-----------\n");
    // print(tmp_tasks);
    // printf("m_pending_task:-----------\n");
    // print(m_pending_task);
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