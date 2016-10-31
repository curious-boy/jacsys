#ifndef XMLCONFIG_H
#define XMLCONFIG_H

#include <string>
#include <iostream>
#include "tinyxml2.h"

using namespace tinyxml2;


class XMLConfig
{
    public:
        bool initByLoadFile();

    public:
        std::string dbServerIp;
            int    port;
            std::string dbName;
            std::string userName;
            std::string password;

            int roundtimeinterval;
            int timeoutofstop;
            int respondtimes;

};
#endif

