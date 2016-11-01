// define struct of
#ifndef PREDEF_H
#define PREDEF_H
#include "preDef.h"

#include <string>

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
    std::string  macId;
	UINT8  unReplyNum;
	UINT16 curMsgSerialNo;

    UINT8  machine_state_has_record;  //当前id的机器状态在数据库中是否有记录 0 1
	UINT8  machine_state;			//机器状态 1 开车 0 停车
	UINT8  halting_reason;			//机器故障信息 0x00 无故障 0x01 断纱 0x02 。。
	UINT32 broken_total_time;		//停机时长，单位：秒

	UINT32 total_run_time;				//开机时长 单位：秒
	UINT32 total_day_time;				//机台当日开机时长
	UINT32 total_day_produced;			//机台当时产量

	std::string figure_name;			//花样名称
	UINT32 latitude;
	UINT32	opening;
	UINT32 tasks_number;
	UINT32 number_produced;
	UINT32	how_long_to_finish;
	UINT16	concurrent_produce_number;
	std::string operator_num;
	UINT32  product_total_time;
	UINT32  product_total_output;

	UINT8  state;
}INFO_Node,*pINFO_Node;


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
} ModifyGateWayDestAddr,*pModifyGateWayDestAddr;






#endif
