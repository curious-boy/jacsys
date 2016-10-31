#include "XMLConfig.h"


#define XML_FILE "jaccfg.xml"

 bool XMLConfig::initByLoadFile()
        {
            XMLDocument cfgdoc;
            cfgdoc.LoadFile(XML_FILE);

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
            roundtimeinterval = atoi(std::string(node_element->FirstChildElement("roundtimeinterval")->GetText()).c_str());
            timeoutofstop = atoi(std::string(node_element->FirstChildElement("timeoutofstop")->GetText()).c_str());
            respondtimes = atoi(std::string(node_element->FirstChildElement("respondtimes")->GetText()).c_str());

            return true;

#if 0
            cout<<"ip: "<<pjacconfig_->dbServerIp.c_str()<<endl;
            cout<<"port: "<<pjacconfig_->port<<endl;
            cout<<"dbName: "<<pjacconfig_->dbName.c_str()<<endl;
            cout<<"userName: "<<pjacconfig_->userName.c_str()<<endl;
            cout<<"password: "<<pjacconfig_->password.c_str()<<endl;

            cout<<"roundtimeinterval: "<<pjacconfig_->roundtimeinterval<<endl;
            cout<<"timeoutofstop: "<<pjacconfig_->timeoutofstop<<endl;
            cout<<"respondtimes: "<<pjacconfig_->respondtimes<<endl;

#endif
        }

