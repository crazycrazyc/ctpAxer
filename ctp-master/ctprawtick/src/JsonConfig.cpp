#include "../include/JsonConfig.h"
#include <fstream>
#include <iostream>
#include <cctype>
#include <vector>
#include <sstream>

using namespace std;

JsonConfig::JsonConfig() {
}

JsonConfig::JsonConfig(const string& filename) {
    LoadFile(filename);
}

bool JsonConfig::LoadFile(const string& filename) {
    m_filename = filename;
    m_Contents.clear();
    
    ifstream file(filename);
    if (!file.is_open()) {
        throw FileNotFound(filename);
    }
    
    string line;
    bool inObject = false;
    
    while (getline(file, line)) {
        line = Trim(line);
        
        // 跳过空行和注释
        if (line.empty() || line[0] == '/' || line[0] == '#') {
            continue;
        }
        
        // 检查是否进入JSON对象
        if (line.find('{') != string::npos) {
            inObject = true;
            continue;
        }
        
        // 检查是否退出JSON对象
        if (line.find('}') != string::npos) {
            inObject = false;
            continue;
        }
        
        // 只有在对象内部才解析键值对
        if (inObject && IsValidJsonLine(line)) {
            string key = ExtractKey(line);
            string value = ExtractValue(line);
            
            if (!key.empty()) {
                m_Contents[key] = value;
            }
        }
    }
    
    file.close();
    return true;
}

bool JsonConfig::KeyExists(const string& key) const {
    return m_Contents.find(key) != m_Contents.end();
}

vector<string> JsonConfig::ReadArray(const string& key) const {
    string value = Read<string>(key);
    return SplitString(value, ',');
}

vector<string> JsonConfig::ReadArray(const string& key, const vector<string>& defaultValue) const {
    if (!KeyExists(key)) {
        return defaultValue;
    }
    return ReadArray(key);
}

// 静态辅助函数实现
string JsonConfig::Trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

string JsonConfig::ExtractKey(const string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == string::npos) {
        return "";
    }
    
    string key = line.substr(0, colonPos);
    key = Trim(key);
    
    // 移除引号
    if (key.length() >= 2 && key[0] == '"' && key[key.length()-1] == '"') {
        key = key.substr(1, key.length()-2);
    }
    
    return key;
}

string JsonConfig::ExtractValue(const string& line) {
    size_t colonPos = line.find(':');
    if (colonPos == string::npos) {
        return "";
    }
    
    string value = line.substr(colonPos + 1);
    value = Trim(value);
    
    // 移除末尾的逗号
    if (!value.empty() && value[value.length()-1] == ',') {
        value = value.substr(0, value.length()-1);
    }
    
    // 移除引号
    if (value.length() >= 2 && value[0] == '"' && value[value.length()-1] == '"') {
        value = value.substr(1, value.length()-2);
    }
    
    return value;
}

bool JsonConfig::IsValidJsonLine(const string& line) {
    string trimmed = Trim(line);
    if (trimmed.empty()) {
        return false;
    }
    
    // 检查是否包含冒号（键值对）
    return trimmed.find(':') != string::npos;
}

vector<string> JsonConfig::SplitString(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    
    while (getline(ss, token, delimiter)) {
        token = Trim(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
} 