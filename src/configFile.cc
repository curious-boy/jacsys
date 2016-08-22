///ConfigFile.cpp  
  
#include "ConfigFile.h"  
  
#define LINE_LEN 256  
  
///public  
ConfigFile::ConfigFile()  
{  
}  
  
ConfigFile::~ConfigFile()  
{  
}  
  
bool ConfigFile::open(const string file_path)  
{  
    bool ret = false;  
  
    if (file_path.empty())  
    {  
        return ret;  
    }  
  
    m_path = file_path;  
    fstream file_in(file_path.c_str(), ios_base::in);  
    if (file_in.is_open())  
    {  
        string str, left, right;  
        unsigned int len, pos;  
        char szLine[LINE_LEN] = {0};  
        while (file_in.getline(szLine, LINE_LEN))  
        {  
            len = strlen(szLine);  
            m_data.push_back(szLine);  
  
            str.assign(szLine, len);  
            trim_left_right(str);  
            if (str[0] != '#')  
            {  
                pos = str.find('=');  
                left = str.substr(0, pos);  
                right = str.substr(pos+1);  
                trim_left_right(left);  
                trim_left_right(right);  
                m_map.insert(pair<string, string>(left, right));  
            }  
              
            bzero(szLine, LINE_LEN);  
        }  
        file_in.close();  
        ret = true;  
    }  
      
    return ret;  
}  
  
bool ConfigFile::save()  
{  
    bool ret = false;  
  
    fstream file_out(m_path.c_str(), ios_base::out);  
    if (file_out.is_open())  
    {  
        vector<string>::iterator iter;  
        for (iter=m_data.begin(); iter!=m_data.end(); ++iter)  
        {  
            string str = (*iter);  
            file_out << str << endl;  
        }  
        file_out.close();  
        ret = true;  
    }  
          
    return ret;  
}  
  
string ConfigFile::get_string(const string key)  
{  
    string str = "";  
    if (!m_map.empty())  
    {  
        str = m_map[key];  
    }  
    return str;  
}  
  
unsigned int ConfigFile::get_int(const string key)  
{  
    unsigned int value = 0;  
  
    string str = get_string(key);  
    value = atoi(str.c_str());  
      
    return value;  
}  
  
bool ConfigFile::set_string(const string key, const string value)  
{  
    bool ret = false;  
  
    if (!m_map.empty())  
    {  
        m_map[key] = value;  
        set_vector_string(key, value);  
        ret = true;  
    }  
      
    return ret;  
}  
  
bool ConfigFile::set_int(const string key, const unsigned int value)  
{  
    bool ret = false;  
      
    if (!m_map.empty())  
    {  
        char szValue[32] = {0};  
        sprintf(szValue, "%u", value);  
        m_map[key] = szValue;  
        set_vector_string(key, szValue);  
        ret = true;  
    }  
      
    return ret;  
}  
  
///private  
void ConfigFile::trim_left_right(string &str)  
{  
    if (!str.empty())  
    {  
        int i;  
        int len = str.size();  
        for (i=0; i<len; i++)  
        {  
            if (str[i] != ' ')  
            {  
                break;  
            }  
        }  
        str.erase(0, i);  
        len = str.size();  
        for (i=len-1; i>0; i--)  
        {  
            if (str[i] != ' ')  
            {  
                break;  
            }  
        }  
        str.erase(i+1, len);  
    }  
}  
  
void ConfigFile::set_vector_string(const string key, const string value)  
{  
    if (!m_data.empty() && !key.empty())  
    {  
        vector<string>::iterator iter;  
        string str;  
        for (iter=m_data.begin(); iter!=m_data.end(); ++iter)  
        {  
            str = *iter;  
            trim_left_right(str);  
            int pos1 = str.find('=');  
              
            int pos2 = (*iter).find('=');  
            int len = (*iter).size();  
  
            string substr = str.substr(0, pos1);  
            trim_left_right(substr);  
            string value2 = " "+value;  
            if (str[0]!='#' && substr==key)  
            {  
                (*iter).replace(pos2+1, len-pos2-1, value2);  
            }  
        }  
    }     
}