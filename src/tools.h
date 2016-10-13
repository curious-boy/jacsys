// tools.h
//#include "MsgTypeDef.h"


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
int GetCurrentTime()
{
	time_t t;
	int j;
	j = time(&t);
	return j;
}