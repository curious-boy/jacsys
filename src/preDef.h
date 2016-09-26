// preDefine.h
#ifndef PREDEFINE_H
#define PREDEFINE_H



typedef uint8_t         UINT8;
typedef uint16_t        UINT16;
typedef uint32_t        UINT32;
typedef uint64_t        UINT64;


#define MAX_SERIAL_NO                       65535          // 最大命令序号，超过后重新排序
#define MAX_UNREPLY_NUM                     3           //节点未响应最大次数
#define  ROUND_INTERVAL_SECONDS        1                 //轮询间隔时间
#define  MAX_THREAD_NUM                     6               //最大线程数目 与支持的网关数有关
#define STRING_MAXLEN                        20             //最大的字符串长度，暂定


//淇敼缃戝叧鐨勭洰鏍囪妭鐐瑰湴鍧�搴旂瓟
typedef struct
{
    UINT8 protocolTag1;
    UINT8 protocolTag2;
    UINT8 protocolTag3;
    UINT8 funcCode;
    UINT8 ackCode;

} RspAck,*pRspAck;

// 淇敼缃戝叧鐨勭洰鏍囪妭鐐瑰湴鍧�
typedef struct
{
    UINT8 protocolTag1;
    UINT8 protocolTag2;
    UINT8 protocolTag3;
    UINT8 funcCode;
    UINT16 addr;
} ModifyGateWayDestAddr,*pModifyGateWayDestAddr;



#endif