#ifndef ROCKET_NET_CODEC_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_CODEC_ABSTRACT_PROTOCOL_H
#include <memory>
#include <string>
namespace rocket{
// 所有协议的基类
/*  注：
    这种继承语句在C++中通常用于实现共享所有权的智能指针。当一个类需要在其成员函数中返回一个指向自身的shared_ptr时，
    可以使用std::enable_shared_from_this类模板来实现。这样可以确保在使用shared_ptr管理对象的情况下，正确地获取指向自身的shared_ptr。
    具体来说，当一个类继承自std::enable_shared_from_this<T>时，它就可以调用shared_from_this()成员函数来获取一个指向自身的shared_ptr。
    这个shared_ptr可以与其他shared_ptr共享所有权，当所有的shared_ptr都被销毁时，对象也会被正确地释放。
    在上述代码中，class AbstractProtocol继承自std::enable_shared_from_this<AbstractProtocol>，这意味着AbstractProtocol类可以调用shared_from_this()函数来获取指向自身的shared_ptr。
    这在某些情况下非常有用，比如当需要在类的成员函数中传递一个指向自身的智能指针时，或者需要在异步操作中传递一个指向自身的智能指针时。
    需要注意的是，使用std::enable_shared_from_this时，必须确保对象是通过shared_ptr进行管理的，否则调用shared_from_this()函数将导致未定义的行为。
    此外，由于std::enable_shared_from_this使用了模板参数T，因此在继承时需要指定具体的派生类类型。
*/
// class AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol>{
struct AbstractProtocol{
private:
    std::string m_request_id;// 请求号，唯一标识 一个请求或者响应
public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;
    AbstractProtocol() = default;
    inline std::string getMsgId()const { return m_request_id; }
    inline void setMsgId(const std::string& req_id) { m_request_id = req_id; }
    virtual ~AbstractProtocol() = default;// base class 之析构函数必须是虚的析构函数
};



}// rocket

#endif // ROCKET_NET_CODEC_ABSTRACT_PROTOCOL_H