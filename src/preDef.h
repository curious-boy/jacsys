// preDefine.h
#ifndef PREDEFINE_H
#define PREDEFINE_H

typedef uint8_t			UINT8;
typedef uint16_t		UINT16;
typedef uint32_t		UINT32;
typedef uint64_t		UINT64;


#define MAX_SERIAL_NO  65535

//修改网关的目标节点地址应答
typedef struct
{
	UINT8 protocolTag1;
	UINT8 protocolTag2;
	UINT8 protocolTag3;
	UINT8 funcCode;
	UINT8 ackCode;	

	
} RspAck,*pRspAck;

// 修改网关的目标节点地址
typedef struct 
{
	UINT8 protocolTag1;
	UINT8 protocolTag2;
	UINT8 protocolTag3;
	UINT8 funcCode;
	UINT16 addr;
}ModifyGateWayDestAddr,*pModifyGateWayDestAddr;



#endif