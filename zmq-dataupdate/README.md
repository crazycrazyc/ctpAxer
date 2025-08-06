
# requirements
1. c++ 实现 zmq 在 本地 端口8888 订阅sub消息
2. 消息来自其他进程的publish
3. 消息分2 frame， frame1 为消息类型 ，frame2 为消息内容
4. 提供接收消息解析函数等待开发这填充
5. 项目使用cmake 构建 
6. linux下运行，连接mysql数据库，使用到 mysqlcppconn 
7. 在当前目录下完成代码
8. 采用类oop设计，增加读取配置参数功能，日志写入功能