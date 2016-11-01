#include "XMLConfig.h"

//#include "tools.h"


#define XML_FILE "jaccfg.xml"

bool XMLConfig::initByLoadFile()
{
    std::string filename=getAbsolutePath();
    filename.append(XML_FILE);

    XMLDocument cfgdoc;
    cfgdoc.LoadFile(filename.c_str());

    XMLElement* root_element = cfgdoc.FirstChildElement( "config" );
    if(root_element == NULL)
    {
        std::cout<<"get root_element failed!"<<std::endl;
        return false;
    }

    XMLElement* database_element = root_element->FirstChildElement("databaseinfo");
    XMLElement* node_element = root_element->FirstChildElement("nodeinfo");

    if(database_element == NULL)
    {
        std::cout<<"get databaseinfo failed!"<<std::endl;
        return false;
    }
    dbServerIp = std::string(database_element->FirstChildElement("dbserver")->GetText());
    port = atoi(std::string(database_element->FirstChildElement("port")->GetText()).c_str());
    dbName = std::string(database_element->FirstChildElement("dbname")->GetText());
    userName = std::string(database_element->FirstChildElement("username")->GetText());
    password = std::string(database_element->FirstChildElement("password")->GetText());

    if(node_element == NULL)
    {
        std::cout<<"get nodeinfo failed!"<<std::endl;
        return false;
    }
    roundtimeinterval = atof(std::string(node_element->FirstChildElement("roundtimeinterval")->GetText()).c_str())/(double)1000;
    if(roundtimeinterval < 0.1)
    {
        roundtimeinterval = 0.1;
    }
    else if(roundtimeinterval >10)
    {
        roundtimeinterval = 10;
    }

    timeoutofstop = atoi(std::string(node_element->FirstChildElement("timeoutofstop")->GetText()).c_str());
    if(timeoutofstop < 1)
    {
        timeoutofstop = 1;
    }
    else if(timeoutofstop > 100000)
    {
        timeoutofstop = 100000;
    }
    respondtimes = atoi(std::string(node_element->FirstChildElement("respondtimes")->GetText()).c_str());
    if(respondtimes < 1)
    {
        respondtimes = 1;
    }
    else if(respondtimes > 10)
    {
        respondtimes = 10;
    }

    return true;
}


#define MAX_PATH 256

std::string XMLConfig::getAbsolutePath()
{
    char tpath[MAX_PATH];

    int cnt = readlink("/proc/self/exe",tpath,MAX_PATH);
    if(cnt <0 || cnt >= MAX_PATH)
    {
        return "";
    }

    int i;
    for(i=cnt;i>=0;--i)
    {
        if(tpath[i]=='/')
        {
            tpath[i+1]='\0';
            break;
        }
    }

    return std::string(tpath);
}



