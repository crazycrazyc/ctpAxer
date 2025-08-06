#include "../include/ConfigManager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

ConfigManager::ConfigManager(const std::string& filename) : configFile(filename) {
}

ConfigManager::~ConfigManager() {
}

bool ConfigManager::loadConfig() {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        std::cerr << "无法打开配置文件: " << configFile << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行和注释行
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // 去除首尾空格
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            config[key] = value;
        }
    }

    file.close();
    return true;
}

std::string ConfigManager::getValue(const std::string& key, const std::string& defaultValue) {
    auto it = config.find(key);
    if (it != config.end()) {
        return it->second;
    }
    return defaultValue;
}

int ConfigManager::getIntValue(const std::string& key, int defaultValue) {
    std::string value = getValue(key);
    if (value.empty()) {
        return defaultValue;
    }
    
    try {
        return std::stoi(value);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

bool ConfigManager::getBoolValue(const std::string& key, bool defaultValue) {
    std::string value = getValue(key);
    if (value.empty()) {
        return defaultValue;
    }
    
    // 转换为小写进行比较
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    return (value == "true" || value == "1" || value == "yes");
} 