cmake_minimum_required(VERSION 3.5.1)

# 项目名称
PROJECT(CTPRawTick)
# 设置编译模式
# set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_BUILD_TYPE "Debug")

# 库头文件
INCLUDE_DIRECTORIES(include)

# 查找源文件
AUX_SOURCE_DIRECTORY(src DIR_SRCS)

#动态库链接目录
LINK_DIRECTORIES("lib")

# 编译选项
SET(CXX_FLAGS -g -Wall)
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXX_FLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXX_FLAGS} -O3 -Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

# 支持C++11
ADD_DEFINITIONS(-std=c++11)

# 查找protobuf
find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

# 设置debug/release输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin/debug/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin/release/)

MESSAGE(STATUS ${DIR_SRCS})
# MESSAGE(STATUS ${ZLOG_LIB})
# MESSAGE(STATUS ${CTP_LIB})

# 目标可执行文件
# ADD_EXECUTABLE(ctprawtickers "${DIR_SRCS}")

# 设置CTP外部库
# TARGET_LINK_LIBRARIES(ctprawtickers zlog;thostmduserapi_se;thosttraderapi_se;zmq ;mysqlclient)

#     "src/CTPTrader.cpp"  "src/appConfig.cpp" "src/JsonConfig.cpp" "src/config.cpp" "src/utils.cpp"
#     "src/appConfig.cpp" "src/ProtobufConverter.cpp" "proto/instrument.pb.cc")

# 添加包含CTPTrader的可执行文件
ADD_EXECUTABLE(ctptrader "src/main.cpp" "src/CTPTrader.cpp" "src/CTPQuote.cpp" 
"src/appConfig.cpp" "src/JsonConfig.cpp" "src/config.cpp" "src/utils.cpp"
"src/ProtobufConverter.cpp" "proto/instrument.pb.cc" "proto/investor_position.pb.cc" "src/ZMQPublisher.cpp")

# 添加数据库连接测试程序

# 添加protobuf测试程序

# 添加protobuf示例程序



# 查找 CTP 库文件
# find_library(CTP_TRADER_LIB thosttraderapi_se 
#     PATHS 
#     /usr/lib 
#     /usr/local/lib 
#     ${CMAKE_SOURCE_DIR}/lib
#     NO_DEFAULT_PATH
# )

# if(NOT CTP_TRADER_LIB)
#     message(FATAL_ERROR "CTP trader library not found. Please install thosttraderapi_se")
# endif()

# 设置CTP外部库

# 为ctptrader设置链接库
TARGET_LINK_LIBRARIES(ctptrader zlog;thosttraderapi_se;thostmduserapi_se;zmq;${Protobuf_LIBRARIES})

# 为数据库连接测试程序设置链接库

# 为protobuf测试程序设置链接库

# 为protobuf示例程序设置链接库

# 添加ZMQ发布者测试程序

# 为ZMQ发布者测试程序设置链接库





# 添加行情订阅程序
ADD_EXECUTABLE(ctpmarket "src/main_market.cpp" "src/CTPQuote.cpp" "src/ZMQPublisher.cpp" "src/appConfig.cpp" "src/utils.cpp" "src/JsonConfig.cpp" "src/ProtobufConverter.cpp" "proto/market_data.pb.cc" "proto/instrument.pb.cc" "proto/investor_position.pb.cc")

# 为行情订阅程序设置链接库
TARGET_LINK_LIBRARIES(ctpmarket zlog;thostmduserapi_se;zmq;${Protobuf_LIBRARIES})

# 添加合约查询程序
ADD_EXECUTABLE(ctpinstrument "src/main_instrument.cpp" "src/CTPTrader.cpp" "src/ZMQPublisher.cpp" "src/appConfig.cpp" "src/utils.cpp" "src/JsonConfig.cpp" "src/ProtobufConverter.cpp" "proto/instrument.pb.cc" "proto/investor_position.pb.cc" "proto/market_data.pb.cc")

# 为合约查询程序设置链接库
TARGET_LINK_LIBRARIES(ctpinstrument zlog;thosttraderapi_se;zmq;${Protobuf_LIBRARIES})

# 添加持仓资金监控程序
ADD_EXECUTABLE(ctpmonitor "src/main_monitor.cpp" "src/CTPTrader.cpp" "src/ZMQPublisher.cpp" "src/appConfig.cpp" "src/utils.cpp" "src/JsonConfig.cpp" "src/ProtobufConverter.cpp" "proto/instrument.pb.cc" "proto/investor_position.pb.cc" "proto/market_data.pb.cc")

# 为持仓资金监控程序设置链接库
TARGET_LINK_LIBRARIES(ctpmonitor zlog;thosttraderapi_se;zmq;${Protobuf_LIBRARIES})

