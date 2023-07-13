# 编译器
CXX = g++

# 编译选项
# CFLAGS = -Wall -Wextra -Werror
CXXFLAGS += -g -O0 -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable
# 目标文件
# TARGET = /home/ubuntu/projects/myTinyRPC/rocket/lib/librocket.a
TARGET = librocket.a

PATH_ROCKET = /home/ubuntu/projects/myTinyRPC/rocket 
PATH_TESTCASES = $(PATH_ROCKET)/testcases

PATH_COMM = $(PATH_ROCKET)/common
PATH_NET = $(PATH_ROCKET)/net 
PATH_NET_TCP = $(PATH_ROCKET)/net/tcp
PATH_NET_CODEC = $(PATH_ROCKET)/net/codec
PATH_NET_RPC = $(PATH_ROCKET)/net/rpc
PATH_PB = $(PATH_ROCKET)/pb
# 源文件
# SRCS = /home/ubuntu/projects/myTinyRPC/rocket/common/log.cpp /home/ubuntu/projects/myTinyRPC/rocket/common/util.cpp  
# SRCS = $(PATH_COMM)/log.cpp $(PATH_COMM)/util.cpp $(PATH_CONF)/config.cpp

SRCS := $(wildcard ../common/*.cpp) $(wildcard ../net/*.cpp) $(wildcard ../net/tcp/*.cpp) $(wildcard ../net/codec/*.cpp) $(wildcard ../net/rpc/*.cpp) $(wildcard ../pb/*.cpp)
# 头文件
# HEADERS = 	$(PATH_COMM)/log.h 		 \ 
# 			$(PATH_COMM)/util.h		 \
# 			$(PATH_COMM)/config.h	 \
# 			$(PATH_COMM)/mutex.h     \
# 			$(PATH_NET)/eventloop.h  \
# 			$(PATH_NET)/wakeup_fd_event.h  \
# 			$(PATH_NET)/fdevent.h  

HEADERS = $(PATH_COMM)/*.h $(PATH_NET)/*.h $(PATH_NET_TCP)/*.h $(PATH_NET_CODEC)/*.h $(PATH_NET_RPC)/*.h $(PATH_PB)/*.h

# 用来生成测试log文件的编译指令
CC2=g++
CFLAGS2 += -g -O0 -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable
CFLAGS2 += -I.
LIBS2 = -L. -lrocket -L/usr/lib -ltinyxml -lpthread -lprotobuf

# 生成目标文件
OBJS = $(SRCS:.cpp=.o)
TEST_TARGETS = test_log test_eventloop test_tcp test_client test_rpc_client test_rpc_server
# 需要编译的目标
all: $(TARGET) $(TEST_TARGETS)
 
# 生成目标文件
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $< 
# 生成静态库文件
$(TARGET): $(OBJS)
	ar rcs $@ $^
# 生成log测试文件,这里使用-g代表可以生成有调试信息的可执行文件test_log！没-g参数你都不可能有这个可执行文件test_log的调试信息
test_log: test_log.cpp
	$(CXX) -g -o test_log test_log.cpp $(CFLAGS2) $(LIBS2)
test_eventloop: test_eventloop.cpp
	$(CXX) -g -o test_eventloop test_eventloop.cpp $(CFLAGS2) $(LIBS2)
test_tcp: test_tcp.cpp
	$(CXX) -g -o test_tcp test_tcp.cpp $(CFLAGS2) $(LIBS2)
test_client: test_client.cpp
	$(CXX) -g -o test_client test_client.cpp $(CFLAGS2) $(LIBS2)
test_rpc_client: test_rpc_client.cpp
	$(CXX) -g -o test_rpc_client test_rpc_client.cpp $(CFLAGS2) $(LIBS2)
# $(CXX) -g -o test_rpc_client test_rpc_client.cpp $(PATH_PB)/order.pb.cpp $(CFLAGS2) $(LIBS2) 
test_rpc_server: test_rpc_server.cpp
	$(CXX) -g -o test_rpc_server test_rpc_server.cpp $(CFLAGS2) $(LIBS2)
# $(CXX) -g -o test_rpc_server test_rpc_server.cpp $(PATH_PB)/order.pb.cpp $(CFLAGS2) $(LIBS2)
# 清理
clean:
	# rm -f $(OBJS) $(TARGET) test_log test_eventloop test_tcp test_rpc_server test_rpc_client $(TEST_TARGETS)
	rm -f $(OBJS) $(TARGET) $(TEST_TARGETS)
