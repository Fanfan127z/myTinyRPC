#include "rpc_closure.h"
#include "../../common/log.h"// use log
namespace rocket{

void RpcClosure::Run(){
    if(m_callback != nullptr){
        m_callback();// directly run callback function
        DEBUGLOG("success run callback function");
    }
}
    
}// rocket