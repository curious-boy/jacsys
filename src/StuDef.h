// define struct of 
#ifndef PREDEF_H
#define PREDEF_H
#include "preDef.h"


// info of Gateway
// typedef struct 
// {
// 	string gatewayId;
// 	TcpConnectionPtr pConn;

// }INFO_Gateway,*pINFO_Gateway;


// info of node 
typedef struct 
{
	UINT16 addr;
	UINT8  unReplyNum;
	UINT16 curMsgSerialNo;
	UINT8  state;
}INFO_Node,*pINFO_Node;

#endif