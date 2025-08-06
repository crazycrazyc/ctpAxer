#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>

class ConfigManager {
private:
    std::map<std::string, std::string> config;
    std::string configFile;

public:
    ConfigManager(const std::string& filename = "config.ini");
    ~ConfigManager();

    bool loadConfig();
    std::string getValue(const std::string& key, const std::string& defaultValue = "");
    int getIntValue(const std::string& key, int defaultValue = 0);
    bool getBoolValue(const std::string& key, bool defaultValue = false);
    
    // 获取常用配置项
    std::string getZMQAddress() { return getValue("zmq.address", "tcp://localhost:8888"); }
    std::string getDBHost() { return getValue("database.host", "localhost"); }
    int getDBPort() { return getIntValue("database.port", 3306); }
    std::string getDBUser() { return getValue("database.user", "root"); }
    std::string getDBPassword() { return getValue("database.password", ""); }
    std::string getDBName() { return getValue("database.name", "test"); }
    std::string getLogFile() { return getValue("log.file", "zmq_subscriber.log"); }
    int getLogLevel() { return getIntValue("log.level", 1); }
};

#endif // CONFIG_MANAGER_H 