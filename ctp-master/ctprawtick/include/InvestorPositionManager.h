#ifndef INVESTOR_POSITION_MANAGER_H
#define INVESTOR_POSITION_MANAGER_H

#include <string>
#include <vector>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <ThostFtdcUserApiStruct.h>
#include <ThostFtdcTraderApi.h>

using namespace std;

// 投资者持仓数据库管理器
class InvestorPositionDBManager {
private:
    sql::mysql::MySQL_Driver *driver;
    sql::Connection *connection;
    string host;
    string user;
    string password;
    string database;
    int port;

public:
    InvestorPositionDBManager(const string& host, const string& user, 
                             const string& password, const string& database, int port);
    ~InvestorPositionDBManager();
    
    // 连接数据库
    bool connect();
    
    // 保存投资者持仓数据到数据库
    bool saveInvestorPositions(const vector<CThostFtdcInvestorPositionField>& positions);
};

// 投资者持仓查询管理器
class InvestorPositionQueryManager {
private:
    CThostFtdcTraderApi *api;
    int requestid;
    bool isConnected;
    bool isLoggedIn;
    
    // 连接参数
    string td_server;
    string brokerid;
    string userid;
    string password;
    
    // 查询结果
    vector<CThostFtdcInvestorPositionField> positionResults;
    
public:
    InvestorPositionQueryManager(const string& td_server, const string& brokerid, 
                                const string& userid, const string& password);
    ~InvestorPositionQueryManager();
    
    // 连接和登录
    bool connect();
    void disconnect();
    
    // 查询持仓
    bool queryInvestorPosition(const string& instrumentID = "");
    
    // 查询持仓并保存到数据库
    bool queryAndSavePositions(const string& db_host, const string& db_user, 
                              const string& db_password, const string& db_database, int db_port);
    
    // 获取查询结果
    vector<CThostFtdcInvestorPositionField> getPositionResults();
    
    // 清空查询结果
    void clearPositionResults();
    
private:
    // 内部状态管理
    void setConnected(bool connected);
    void setLoggedIn(bool loggedIn);
};

#endif // INVESTOR_POSITION_MANAGER_H 