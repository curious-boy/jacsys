///ConfigFile.h 配置文件操作  
///格式如下：  
/************************************************* 
#this is comment 
name = frankz 
date =  
**************************************************/      
  
#ifndef CONFIG_FILE_H_  
#define CONFIG_FILE_H_  
  
#include <iostream>  
#include <fstream>  
#include <string.h>  
#include <stdlib.h>  
#include <map>  
#include <vector>  
  
using namespace std;  
  
class ConfigFile  
{  
public:  
    ConfigFile();  
    ~ConfigFile();  
  
    bool open(const string file_path);  
  
    bool save();  
  
    string get_string(const string key);  
  
    unsigned int get_int(const string key);  
  
    bool set_string(const string key, const string value);  
  
    bool set_int(const string, const unsigned int value);  
  
private:  
    void trim_left_right(string &str);  
    void set_vector_string(const string key, const string value);  
  
private:  
    string m_path;  
    vector<string> m_data;  
    map<string, string> m_map;  
};  
  
#endif // CONFIG_FILE_H  
