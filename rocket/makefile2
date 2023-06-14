# PATH_BIN = bin
# PATH_LIB = lib 
# PATH_OBJ = obj 

# PATH_ROCKET = rocket 
# PATH_COMM = $(PATH_ROCKET)/common
# PATH_NET = $(PATH_ROCKET)/net 

# PATH_TESTCASES = testcases 

# # will install lib to /usr/lib/libsocket.a
# PATH_INSTALL_LIB_ROOT = /usr/lib 

# # will install all header file to /usr/include/socket
# PATH_INSTALL_INC_ROOT = /usr/include 

# PATH_INSTALL_INC_COMM =  $(PATH_INSTALL_INC_ROOT)/$(PATH_COMM) 
# PATH_INSTALL_INC_NET =  $(PATH_INSTALL_INC_ROOT)/$(PATH_NET) 

# # PATH_PROTOBUF = /usr/include/google
# # PATH_TINYXML = /usr/include/tinyxml

# CXX := g++

# CXXFLAGS += -g -O0 -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable

# CXXFLAGS += -I./ -I$(PATH_ROCKET) -I$(PATH_COMM) -I$(PATH_NET)

# LIBS += /usr/lib/libprotobuf.a / usr/lib/libtinyxml.a 

# COMM_OBJ = $(patsubst $(PATH_COMM)/%.cpp, $(PATH_OBJ)%.o, $(wildcard $(PATH_COMM)/*.cpp))
# NET_OBJ = $(patsubst $(PATH_NET)/%.cpp, $(PATH_OBJ)%.o, $(wildcard $(PATH_NET)/*.cpp))

# ALL_TESTS : $(PATH_BIN)/test_log

# TEST_CASE_OUT := $(PATH_BIN)/test_log

# LIB_OUT := $(PATH_LIB)/librocket.a

# $(PATH_BIN)/test_log : $(LIB_OUT)
# 	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_log.cpp -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread 


# # 目标文件
# TARGET = /home/ubuntu/projects/myTinyRPC/rocket/lib/librocket.a

# # 头文件
# HEADERS = /home/ubuntu/projects/myTinyRPC/rocket/common/log.h /home/ubuntu/projects/myTinyRPC/rocket/common/util.h

# # 生成静态库文件
# $(TARGET): $(COMM_OBJ)
# 	ar rcs $@ $^

# # 生成目标文件
# %.o: %.c $(HEADERS)
# 	$(CXX) $(CFLAGS) -c -o $@ $< 

##################################
# makefile
# ikerli
# 2022-05-23
##################################

PATH_BIN = bin
PATH_LIB = lib
PATH_OBJ = obj

PATH_ROCKET = rocket
PATH_COMM = $(PATH_ROCKET)/common
PATH_NET = $(PATH_ROCKET)/net
PATH_TCP = $(PATH_ROCKET)/net/tcp
PATH_CODER = $(PATH_ROCKET)/net/coder
PATH_RPC = $(PATH_ROCKET)/net/rpc

PATH_TESTCASES = testcases

# will install lib to /usr/lib/librocket.a
PATH_INSTALL_LIB_ROOT = /usr/lib

# will install all header file to /usr/include/rocket
PATH_INSTALL_INC_ROOT = /usr/include

PATH_INSTALL_INC_COMM = $(PATH_INSTALL_INC_ROOT)/$(PATH_COMM)
PATH_INSTALL_INC_NET = $(PATH_INSTALL_INC_ROOT)/$(PATH_NET)
PATH_INSTALL_INC_TCP = $(PATH_INSTALL_INC_ROOT)/$(PATH_TCP)
PATH_INSTALL_INC_CODER = $(PATH_INSTALL_INC_ROOT)/$(PATH_CODER)
PATH_INSTALL_INC_RPC = $(PATH_INSTALL_INC_ROOT)/$(PATH_RPC)


# PATH_PROTOBUF = /usr/include/google
# PATH_TINYXML = /usr/include/tinyxml

CXX := g++

CXXFLAGS += -g -O0 -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable

CXXFLAGS += -I./ -I$(PATH_ROCKET)	-I$(PATH_COMM) -I$(PATH_NET) -I$(PATH_TCP) -I$(PATH_CODER) -I$(PATH_RPC)

LIBS += /usr/lib/libprotobuf.a	/usr/lib/libtinyxml.a


COMM_OBJ := $(patsubst $(PATH_COMM)/%.cpp, $(PATH_OBJ)/%.o, $(wildcard $(PATH_COMM)/*.cpp))
NET_OBJ := $(patsubst $(PATH_NET)/%.cpp, $(PATH_OBJ)/%.o, $(wildcard $(PATH_NET)/*.cpp))
TCP_OBJ := $(patsubst $(PATH_TCP)/%.cpp, $(PATH_OBJ)/%.o, $(wildcard $(PATH_TCP)/*.cpp))
CODER_OBJ := $(patsubst $(PATH_CODER)/%.cpp, $(PATH_OBJ)/%.o, $(wildcard $(PATH_CODER)/*.cpp))
RPC_OBJ := $(patsubst $(PATH_RPC)/%.cpp, $(PATH_OBJ)/%.o, $(wildcard $(PATH_RPC)/*.cpp))

ALL_TESTS : $(PATH_BIN)/test_log $(PATH_BIN)/test_eventloop $(PATH_BIN)/test_tcp $(PATH_BIN)/test_client $(PATH_BIN)/test_rpc_client $(PATH_BIN)/test_rpc_server

TEST_CASE_OUT := $(PATH_BIN)/test_log $(PATH_BIN)/test_eventloop $(PATH_BIN)/test_tcp $(PATH_BIN)/test_client  $(PATH_BIN)/test_rpc_client $(PATH_BIN)/test_rpc_server

LIB_OUT := $(PATH_LIB)/librocket.a

$(PATH_BIN)/test_log: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_log.cpp -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

# $(PATH_BIN)/test_eventloop: $(LIB_OUT)
# 	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_eventloop.cpp -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

# $(PATH_BIN)/test_tcp: $(LIB_OUT)
# 	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_tcp.cpp -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

# $(PATH_BIN)/test_client: $(LIB_OUT)
# 	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_client.cpp -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

# $(PATH_BIN)/test_rpc_client: $(LIB_OUT)
# 	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_rpc_client.cpp $(PATH_TESTCASES)/order.pb.cpp -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

# $(PATH_BIN)/test_rpc_server: $(LIB_OUT)
# 	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_rpc_server.cpp $(PATH_TESTCASES)/order.pb.cpp -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread


$(LIB_OUT): $(COMM_OBJ) $(NET_OBJ) $(TCP_OBJ) $(CODER_OBJ) $(RPC_OBJ)
	# cd $(PATH_OBJ) && ar rcv librocket.a *.o && cp librocket.a ../lib/
	cd $(PATH_OBJ) && ar rcv librocket.a *.o 
$(PATH_OBJ)/%.o : $(PATH_COMM)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


$(PATH_OBJ)/%.o : $(PATH_NET)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_TCP)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_CODER)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_RPC)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# print something test
# like this: make PRINT-PATH_BIN, and then will print variable PATH_BIN
PRINT-% : ; @echo $* = $($*)


# to clean 
clean :
	rm -f $(COMM_OBJ) $(NET_OBJ) $(TESTCASES) $(TEST_CASE_OUT) $(PATH_LIB)/librocket.a $(PATH_OBJ)/librocket.a $(PATH_OBJ)/*.o

# install
install:
	mkdir -p $(PATH_INSTALL_INC_COMM) $(PATH_INSTALL_INC_NET) $(PATH_INSTALL_INC_TCP) $(PATH_INSTALL_INC_CODER) $(PATH_INSTALL_INC_RPC)\
		&& cp $(PATH_COMM)/*.h $(PATH_INSTALL_INC_COMM) \
		&& cp $(PATH_NET)/*.h $(PATH_INSTALL_INC_NET) \
		&& cp $(PATH_TCP)/*.h $(PATH_INSTALL_INC_TCP) \
		&& cp $(PATH_CODER)/*.h $(PATH_INSTALL_INC_CODER) \
		&& cp $(PATH_RPC)/*.h $(PATH_INSTALL_INC_RPC) \
		&& cp $(LIB_OUT) $(PATH_INSTALL_LIB_ROOT)/


# uninstall
uninstall:
	rm -rf $(PATH_INSTALL_INC_ROOT)/ROCKET && rm -f $(PATH_INSTALL_LIB_ROOT)/librocket.a