# 编译器
CXX = g++

# 编译选项
CFLAGS = -Wall -Wextra -Werror

# 目标文件
# TARGET = /home/ubuntu/projects/myTinyRPC/rocket/lib/librocket.a
TARGET = librocket.a

PATH_ROCKET = /home/ubuntu/projects/myTinyRPC/rocket 
PATH_COMM = $(PATH_ROCKET)/common
PATH_NET = $(PATH_ROCKET)/net 


# 源文件
# SRCS = /home/ubuntu/projects/myTinyRPC/rocket/common/log.cpp /home/ubuntu/projects/myTinyRPC/rocket/common/util.cpp  
# SRCS = $(PATH_COMM)/log.cpp $(PATH_COMM)/util.cpp $(PATH_CONF)/config.cpp

SRCS := $(wildcard /home/ubuntu/projects/myTinyRPC/rocket/common/*.cpp)
# 头文件
HEADERS = $(PATH_COMM)/log.h $(PATH_COMM)/util.h $(PATH_COMM)/config.h $(PATH_COMM)/mutex.h

# 用来生成测试log文件的编译指令
CC2=g++
CFLAGS2=-I.
LIBS2=-L. -lrocket -L/usr/lib -ltinyxml -lpthread

# 生成目标文件
OBJS = $(SRCS:.cpp=.o)

# 需要编译的目标
all: $(TARGET) test_log
 
# 生成目标文件
%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) -c -o $@ $< 
# 生成静态库文件
$(TARGET): $(OBJS)
	ar rcs $@ $^
# 生成log测试文件,这里使用-g代表可以生成有调试信息的可执行文件test_log！没-g参数你都不可能有这个可执行文件test_log的调试信息
test_log: test_log.cpp
	$(CC2) -g -o test_log test_log.cpp $(CFLAGS2) $(LIBS2)
# 清理
clean:
	rm -f $(OBJS) $(TARGET) test_log
