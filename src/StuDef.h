// define struct of 
#include "preDef.h"

// info of gateway
typedef struct 
{
	string gatewayId;
	TcpConnectionPtr pConn;

}INFO_Gateway,*pINFO_Gateway;


// info of node 
typedef struct 
{
	UINT16 addr;
	UINT8  unReplyNum;
	UINT8  state;
}INFO_Node,*pINFO_Node;