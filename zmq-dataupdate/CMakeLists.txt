cmake_minimum_required(VERSION 3.10)
project(zmq_dataupdate)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找必要的包
find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)
pkg_check_modules(CPPZMQ REQUIRED libzmq)

# MySQL Connector/C++
# 手动指定MySQL Connector/C++的路径
set(MYSQLCPPCONN_INCLUDE_DIRS "/usr/include/mysql-cppconn-8")
set(MYSQLCPPCONN_LIBRARIES "/usr/lib/x86_64-linux-gnu/libmysqlcppconn.so")

# 包含目录
include_directories(${ZMQ_INCLUDE_DIRS})
include_directories(${CPPZMQ_INCLUDE_DIRS})
include_directories(${MYSQLCPPCONN_INCLUDE_DIRS})

# 源文件
set(COMMON_SOURCES
    src/ZMQSubscriber.cpp
    src/ConfigManager.cpp
    src/Logger.cpp
    src/DatabaseManager.cpp
    src/InstrumentConverter.cpp
    src/InvestorPositionConverter.cpp
    src/MarketDataConverter.cpp
    src/TradingAccountConverter.cpp
    proto/market_data.pb.cc
)

# 创建原始的数据订阅器可执行文件
add_executable(${PROJECT_NAME} src/main.cpp ${COMMON_SOURCES})

# 创建行情数据订阅器可执行文件
add_executable(zmq_market_subscriber src/main_market.cpp ${COMMON_SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME} 
    ${ZMQ_LIBRARIES}
    ${CPPZMQ_LIBRARIES}
    ${MYSQLCPPCONN_LIBRARIES}
)

target_link_libraries(zmq_market_subscriber 
    ${ZMQ_LIBRARIES}
    ${CPPZMQ_LIBRARIES}
    ${MYSQLCPPCONN_LIBRARIES}
    ${Protobuf_LIBRARIES}
)

# 编译选项
target_compile_options(${PROJECT_NAME} PRIVATE ${ZMQ_CFLAGS_OTHER})
target_compile_options(${PROJECT_NAME} PRIVATE ${CPPZMQ_CFLAGS_OTHER}) 

target_compile_options(zmq_market_subscriber PRIVATE ${ZMQ_CFLAGS_OTHER})
target_compile_options(zmq_market_subscriber PRIVATE ${CPPZMQ_CFLAGS_OTHER}) 