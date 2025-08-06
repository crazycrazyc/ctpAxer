#ifndef JSONCONFIG_H
#define JSONCONFIG_H

#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

class JsonConfig {
private:
    std::map<std::string, std::string> m_Contents;
    std::string m_filename;

    // 辅助函数
    static std::string Trim(const std::string& str);
    static std::string ExtractValue(const std::string& line);
    static std::string ExtractKey(const std::string& line);
    static bool IsValidJsonLine(const std::string& line);
    static std::vector<std::string> SplitString(const std::string& str, char delimiter);

public:
    JsonConfig();
    JsonConfig(const std::string& filename);
    
    // 读取配置项
    template<typename T>
    T Read(const std::string& key) const;
    
    template<typename T>
    T Read(const std::string& key, const T& defaultValue) const;
    
    // 读取数组配置项（逗号分隔的字符串）
    std::vector<std::string> ReadArray(const std::string& key) const;
    std::vector<std::string> ReadArray(const std::string& key, const std::vector<std::string>& defaultValue) const;
    
    // 检查键是否存在
    bool KeyExists(const std::string& key) const;
    
    // 加载配置文件
    bool LoadFile(const std::string& filename);
    
    // 异常类
    struct FileNotFound {
        std::string filename;
        FileNotFound(const std::string& filename_ = std::string()) : filename(filename_) {}
    };
    
    struct KeyNotFound {
        std::string key;
        KeyNotFound(const std::string& key_ = std::string()) : key(key_) {}
    };
};

// 模板特化实现
template<typename T>
T JsonConfig::Read(const std::string& key) const {
    auto it = m_Contents.find(key);
    if (it == m_Contents.end()) {
        throw KeyNotFound(key);
    }
    
    std::istringstream iss(it->second);
    T value;
    iss >> value;
    return value;
}

template<typename T>
T JsonConfig::Read(const std::string& key, const T& defaultValue) const {
    auto it = m_Contents.find(key);
    if (it == m_Contents.end()) {
        return defaultValue;
    }
    
    std::istringstream iss(it->second);
    T value;
    iss >> value;
    return value;
}

// 字符串特化
template<>
inline std::string JsonConfig::Read<std::string>(const std::string& key) const {
    auto it = m_Contents.find(key);
    if (it == m_Contents.end()) {
        throw KeyNotFound(key);
    }
    return it->second;
}

template<>
inline std::string JsonConfig::Read<std::string>(const std::string& key, const std::string& defaultValue) const {
    auto it = m_Contents.find(key);
    if (it == m_Contents.end()) {
        return defaultValue;
    }
    return it->second;
}

#endif // JSONCONFIG_H 