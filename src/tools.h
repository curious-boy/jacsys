// tools.h
//#include "MsgTypeDef.h"

#include<string>



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
    strftime(tBuf,128,"%Y-%m-%d %H:%m:%S",timeinfo);

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
