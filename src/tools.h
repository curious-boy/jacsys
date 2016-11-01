// tools.h
//#include "MsgTypeDef.h"
#ifndef TOOLS_H
#define TOOLS_H

#include<string>

#include "preDef.h"



UINT16 CalcCRC16(UINT16 crc16,UINT8* pData,UINT32 uLen)
{
    unsigned int i;

    while(uLen--)
    {
        crc16=crc16 ^ (unsigned short)(*pData) << 8;
        pData++;
        for (i = 0; i < 8; ++i)
        {
            if (crc16 & 0x8000)
            {
                crc16=crc16<<1 ^ 0x1021;
            }
            else
            {
                crc16=crc16<<1;
            }
        }
    }

        return crc16;
}


// get current time  timestamp
std::string GetCurrentTime()
{
	time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char tBuf[128]={0};
    strftime(tBuf,128,"%Y-%m-%d %H:%M:%S",timeinfo);

	return std::string(tBuf);
}

// get current date
std::string GetCurrentDate()
{
	time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char tBuf[128]={0};
    strftime(tBuf,128,"%Y-%m-%d",timeinfo);

	return std::string(tBuf);
}

int getIndexOfSubMem(char* pchar)
{
    if(pchar == NULL)
    {
        return -1;
    }

    char subpchar[4]={0xDE,0xDF,0xEF,0xD2};

    char* reschar=strstr(pchar,subpchar);

    if(reschar == NULL)
    {
        return -1;
    }
    else
    {
        return reschar-pchar;
    }


    return -1;
}

#if 1
#define MAX_PATH 256

std::string getAbsolutePath()
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

#endif

#endif
