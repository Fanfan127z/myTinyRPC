# 编译器
CXX = g++

# 编译选项
CFLAGS = -Wall -Wextra -Werror

# 目标文件
TARGET = /home/ubuntu/projects/myTinyRPC/rocket/lib/librocket.a

# 源文件
SRCS = /home/ubuntu/projects/myTinyRPC/rocket/common/log.cpp /home/ubuntu/projects/myTinyRPC/rocket/common/util.cpp  

# 头文件
HEADERS = /home/ubuntu/projects/myTinyRPC/rocket/common/log.h /home/ubuntu/projects/myTinyRPC/rocket/common/util.h

# 生成目标文件
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 生成静态库文件
$(TARGET): $(OBJS)
	ar rcs $@ $^

# 生成目标文件
%.o: %.c $(HEADERS)
	$(CXX) $(CFLAGS) -c -o $@ $< 

# 清理
clean:
	rm -f $(OBJS) $(TARGET)
