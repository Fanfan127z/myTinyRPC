// #include "../common/util.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include <algorithm>
#include <sstream>
#include <queue>
// #include <thread> g++编译的时候要加上 -lpthread参数！
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
void pareseServiceFullName(const std::string& full_name, std::string& service_name, std::string& method_name){
    int idx = 0;
    for(int i = 0;i < full_name.size();++i){
        if(full_name[i] == '.'){
            idx = i;
            break;
        }
    }
    service_name = std::string(full_name.begin(), full_name.begin() + idx);
    method_name = std::string(full_name.begin() + idx + 1, full_name.end());
}
void pareseServiceFullName2(const std::string& full_name
                , std::string& service_name, std::string& method_name){
    size_t idx = full_name.find_first_of(".");
    if(idx == full_name.npos){
        printf("not find '.' in full_name[%s]\n", full_name.c_str());
        return;
    }
    service_name = std::string(full_name.begin(), full_name.begin() + idx);
    method_name = std::string(full_name.begin() + idx + 1, full_name.end());
    printf("success parse service_name[%s], method_name[%s] from full_name[%s]\n"
            , service_name.c_str(), method_name.c_str(), full_name.c_str());
}
void tt2(const std::string& addr){// example: addr = "127.0.0.1:9999"
    int idx = addr.find_first_of(":");
    if(idx == addr.npos){
        // ERRORLOG("[%s] is an invalid IPv4 addr", addr.c_str());
        printf("[%s] is an invalid IPv4 addr\n", addr.c_str());
        return;
    }
    std::string m_ip = addr.substr(0, idx);
    uint16_t m_port = std::stoi(addr.substr(idx+1, addr.size() - idx - 1));
    printf("[%s:%d] is an valid IPv4 addr\n", m_ip.c_str(), m_port);
}
void tt(const char* buf, int size){
    vector<char> m_buffer{'a','b','c'};
    m_buffer.resize(10);
    int idx = 0;
    int m_writeIndex = 3;
    // while(idx < size){
    //     m_buffer[m_writeIndex++] = buf[idx++];
    // }
    memcpy(&m_buffer[m_writeIndex], buf, size);
    for_each(m_buffer.begin(), m_buffer.end(), [](int val){
        printf("[%c]\t", val);
    });
    printf("m_writeIndex = [%d]\n", m_writeIndex);

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

class AA{
public:
    static int m_sa;
    AA(){}
    int getSA1() const { return m_sa; }
    static int getSA2() { return m_sa; }
    ~AA(){}
};
int AA::m_sa {0};
void f(){cout <<"f=666"<<endl;}
typedef void(*Fptr)();
enum TimerEventType{
    NORMAL_TIMER_EVENT, // 正常timerEvent定时任务事件，不重复执行（默认）
    REPEATABLE_TIMER_EVENT,// 需要被重复执行的timerEvent定时任务事件
};
struct Base{
    Base(){ cout << "Base()" << endl; }
    virtual ~Base(){ cout << "~Base()" << endl; }
};
struct Paisheng : public Base{
    Paisheng(){ cout << "Paisheng()" << endl; }
    ~Paisheng(){ cout << "~Paisheng()" << endl; }
};
int main(){
    std::string service_name, func_name;
    pareseServiceFullName2("service_name.func_name", service_name, func_name);
    printf("service_name=[%s], func_name=[%s]\n", service_name.c_str(), func_name.c_str());
    // 对于 子类 和 父类 的继承与派生的关系来说，一定是先 构造基类的对象，再构造子类的对象，然后析构的顺序与构造的顺序完全相反！
    // 也即是，先析构子类对象，再析构其父类对象，如果还有更多层的继承与派生关系的话，也以此类推。。。
    // Paisheng p1;
    // cout << "--------------" << endl; 
    // Base b1;
    // cout << "--------------" << endl; 
    // Base * pb = new Paisheng();
    // delete pb;
    // cout << "--------------" << endl; 
    // std::string str;
    // char ch;

    // std::cout << "请输入一行字符串：";
    // std::getline(std::cin, str);

    // std::cout << "请输入一个字符：";
    // std::cin >> ch;

    // std::cout << "输入的字符串为：" << str << std::endl;
    // std::cout << "输入的字符为：" << ch << std::endl;
    // for(char& c : str)c = tolower(c);
    // ch = tolower(ch);
    // std::cout << "输入的字符串为：" << str << std::endl;
    // std::cout << "输入的字符为：" << ch << std::endl;
    // char str[5000];
    // cin.getline(str, 5000);
    // char ch;
    // cin.getline(&ch,1);
    // printf("str=[%s],strlen(str)=[%d],ch=[%c]\n",str,strlen(str),ch);
    // tt("lzf", 3);
    // tt2("127.0.0.1:9999");// example: addr = "127.0.0.1:9999"
    // TimerEventType tt=REPEATABLE_TIMER_EVENT;
    // if(tt==REPEATABLE_TIMER_EVENT){
    //     printf("tt==REPEATABLE_TIMER_EVENT\n");
    // }

    // Fptr alias = f;
    // f();
    // alias();
    // int a =110;
    // cout << "a = "<<a<<",&a = "<<&a<<endl;
    // int&& ba=std::move(a);
    // cout << "----------\nba = "<<ba<<",&ba = "<<&ba<<endl;
    // cout << "a = "<<a<<",&a = "<<&a<<endl;
    // a=2;
    //  cout << "a = "<<a<<",&a = "<<&a<<endl;
    // AA aa;
    // aa.m_sa += 1;
    // printf("aa.getSA1()=[%d]\n", aa.getSA1());
    // printf("aa.getSA2()=[%d]\n", aa.getSA2());
    // aa.m_sa += 2;
    // printf("aa.getSA1()=[%d]\n", aa.getSA1());
    // printf("aa.getSA2()=[%d]\n", aa.getSA2());
    // aa.m_sa += 3;
    // printf("aa.getSA1()=[%d]\n", aa.getSA1());
    // printf("aa.getSA2()=[%d]\n", aa.getSA2());
    // 接下来我要test一下，是否类中的static对象是属于all类的对象的，都是只有一份的,经过我的test，确实是这样子的！


    // std::thread t1(run1);
    // t1.join();
    // std::thread t2(run2);
    // t2.join();
    // print_g_a();
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