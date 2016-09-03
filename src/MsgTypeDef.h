/*************************************************
日期：20160722
起草：Gordon
修改记录：
1、
*************************************************/
////////////////////////////////////////////////////////报文定义开始///////////////////////////////////////////////////////////////////////////
/*
1、自定义消息类型，分为服务器到节点与节点到服务器两大类，
   当请求方发起数据请求时，应答消息类型Msg最高位置1，表示应答帧
2、通讯过程，节点上电后会主动发起注册消息，直到收到服务器的ACK为止，收到ACK表示节点入网成功
3、节点注册后，服务器会向节点请求各种配置信息
4、产量、心跳、机器状态等可定时上传，也可由服务器轮询获取，看配置而定，看实际组网效率
*/
#ifndef MSG_TYPE_DEF_H
#define MSG_TYPE_DEF_H
#include "preDef.h"

//node TO server
#define MSG_LOGIN        0X01//注册
#define MSG_LOGOUT       0X02//注销
#define MSG_HEARTBEAT    0X03//节点心跳
#define MSG_PRODUCTION   0X04//产量
#define MSG_TASKINFO     0X05//班组任务
#define MSG_MACSTATE     0X06//机器状态信息
#define MSG_PATPARA_REPORT 0X07//上报花样信息
#define MSG_MACINFO_REPORT 0x08//上报机器信息
#define MSG_MAXNODEID    0X3F//节点到服务器最大消息ID 

//server TO node
#define MSG_SETMACINFO           0X40  //设置机器信息
#define MSG_SETPATPARA           0X41  //下传花样信息
#define MSG_FIGUREFILE        0X42  //送花样文件
#define MSG_GETMACINFO        0X43  //获取机器信息
#define MSG_GETTASKINFO       0X44  //获取生产任务
#define MSG_GETPRODUCTION     0X45  //获取产量
#define MSG_GETFIRMWAREINFO   0X46  //获取固件信息等
#define MSG_SETINTERVAL       0X47  //设置节点定时消息时间间隔
#define MSG_SETTIME			  		0X48  //设置节点时间
#define MSG_GETMACSTATE			  0X49  //获取机器状态信息
#define MSG_GETPATPARA		  	0X4A  //获取花样信息
//双向通用
#define MSG_COMACK		  0XF0  //通用ack

//消息类型bit7定义
#define MSG_REQUEST (0<<7)
#define MSG_REPLY   (1<<7)

//AckCode定义
#define ACK_OK       0x00  //应答OK
#define ACK_FAIL     0x01  //应答失败
#define ACK_CRCERR   0x02  //crc校验错误
#define ACK_DATALOSS 0x03  //数据包不完整
#define ACK_NOT_LOGIN   0X04//没有注册
#define ACK_MSG_ERROR    0x05  //消息格式或者内容错误
#define ACK_NOT_SUPPORT_TYPE    0x06  //应答命令不支持
#define ACK_OUTOFMEM    0x07  //存储空间不足
#define ACK_FILEBREAK    0x08  //文件不完整


//帧控制信息
#define COM_FRM_HEAD 0xaa//串口帧固定帧第一字节为0xaa
#define COM_FRM_END 0X55//帧结尾固定0x55
#define ComFrameLen 16//一个完整com帧为16字节
#define ExtFrm 0x01//扩展帧
#define StdFrm 0x00//标准帧
#define NotRmtFrm 0x00//非远程帧
#define CanDataSize 8
#define MsgSize 512//
#define HostAddr 0x2002//主机地址初始值，可变
#define STRING_MAXLEN       128             //最大的字符串长度，暂定

#pragma pack(1)    //设置1字节对齐

//传输数据采用大端格式，大小端转换
#define Tranverse16(X)                 ((((UINT16)(X) & 0xff00) >> 8) |(((UINT16)(X) & 0x00ff) << 8))
#define Tranverse32(X)                 ((((UINT32)(X) & 0xff000000) >> 24) | \
(((UINT32)(X) & 0x00ff0000) >> 8) | \
(((UINT32)(X) & 0x0000ff00) << 8) | \
(((UINT32)(X) & 0x000000ff) << 24))

//--通讯定时事件
#define FIRSTEVENT				0		// 第一个事件
#define EVENT_SendMsg			0		// 发送消息等待应答
#define EVENT_CanFrm		    1		// 一个完整的can帧
#define EVENT_Packet			2		// 应用报文接收完整性
#define EVENT_Login				3		// 注册
#define EVENT_PRODUCTION		4		// 报告产量
#define EVENT_PARAREPORT		5//报告参数
#define EVENT_HMI_RECCMD		6//HMI接收命令
#define LASTEVENT				6		// 最后一个事件

#define  PRODUCTION_TIMER 10                           //秒
#define  LOGIN_TIMER  3                                     //秒
#define  ROUND_INTERVAL_SECONDS  0.5             //轮询间隔时间
#define  MAX_THREAD_NUM             6                   //最大线程数目 与支持的网关数有关


//网关标识,可变
#define  GW_ID "HuiGuan-001"

#define ProtocolVer  0

//秒定时器相关参数
#define TMR_START 1
#define TMR_STOP  0
#define TMR_OUT   0XFF

typedef struct  
{
	UINT8 TimerFlg;//定时器标志0停止 1启动 0xff时间到
	UINT16 TimerCnt;//计时器
}TimeEvent,*pTimeEvent;

//--can网络接口
typedef struct  
{
	UINT8 Saddr;//can源地址
	UINT8 Daddr;//can目标地址，最后一次连接的地址
	UINT8 LinkState;//当前链接状态 0未入网 1注册成功 
	UINT8 SendingMsg;//正在发送的消息，等ACK
	UINT8 ReceiveMsgACK;//接收到的ACK
	UINT16 serialNo;//消息序号，从0开始顺序增长，到最大值回到0。
	UINT16 replyNo;	//应答序号，只出现在应答消息中，用于匹配请求消息	
}CanNetif,*pCanNetif;

//--ZigBee网络接口
typedef struct  
{
	UINT16 IdLen;
	char gwID[STRING_MAXLEN];
	UINT8 chan;             /**< \brief 通道号        */
  UINT16 panid;         /**< \brief PanID         */
	UINT16 Saddr;//can源地址
	UINT16 Daddr;//can目标地址，最后一次连接的地址
	UINT8 LinkState;//当前链接状态 0未入网 1注册成功 
	UINT8 SendingMsg;//正在发送的消息，等ACK
	UINT8 ReceiveMsgACK;//接收到的ACK
	UINT16 serialNo;//消息序号，从0开始顺序增长，到最大值回到0。
	UINT16 replyNo;	//应答序号，只出现在应答消息中，用于匹配请求消息	
	UINT8   Interval;//主动上传间隔
}ZigNetif,*pZigNetif;

typedef struct  
{	
	UINT8 SendingMsg;//正在发送的消息，等ACK	
	UINT16 serialNo;//消息序号，从0开始顺序增长，到最大值回到0。	
}SendmsgQ,*pSendmsgQ;


//---------------------------------------------------------------------------------------
// can底层消息格式定义，使用can转uart模块
//---------------------------------------------------------------------------------------
//--单个can帧定义
typedef struct  
{
	UINT8 Flg;//can帧信息，bit7标准帧、扩展帧,bit6数据帧、远程帧,bit3:bit0有效数据长度
	union{
	UINT8 Pcnt;//can帧计数器，bit4：bit0表示第几帧
	struct{
		UINT8 FrmCnt:5;//帧计数器		
		UINT8 bit75:3;
	}str;
	}uni;
	UINT8 Msg;//消息类型，bit7=0表示请求，bit7=1表示应答
	UINT8 Daddr;//can目标地址
	UINT8 Saddr;//can源地址
	UINT8 Data[8];//负载
}CanFrame,*pCanFrame;

//--单个串口帧格式
typedef struct  
{
	UINT8 Head;//固定为0xAA
	UINT8 StdExtFlg;//标准帧、扩展帧、
	UINT8 DatRmtFlg;//数据帧、远程帧
	UINT8 DatLen;//有效数据长度 max=8
	union{
		UINT8 Pcnt;//can帧计数器，bit4：bit0表示第几帧，从0到31,理论最大包32*8=256byte
		struct{
			UINT8 FrmCnt:5;			
			UINT8 bit75:3;//帧计数器
		}str;
	}uni0;
	UINT8 Msg;//消息类型，bit7=0表示请求，bit7=1表示应答
	UINT8 Daddr;//can目标地址
	UINT8 Saddr;//can源地址
	union{
		UINT8 Data[8];//负载
		struct  
		{			
			UINT16 length;//消息长度，包括消息头和消息体
			UINT16 serialNo;//消息序号，从0开始顺序增长，到最大值回到0。
			UINT16 replyNo;	//应答序号，只出现在应答消息中，用于匹配请求消息	
			UINT16 crc16;//CRC校验 为0时做校验
		}str;	
	}uni1;	
}ComFrame,*pComFrame;

//--CAN帧头
typedef struct  
{
	UINT8 Flg;//can帧信息，bit7标准帧、扩展帧,bit6数据帧、远程帧,bit3:bit0有效数据长度 为0时做校验
	union{
		UINT8 Pcnt;//can帧计数器 为0时做校验
		struct{
			UINT8 FrmCnt:5;//帧计数器 0到31 32*8=256字节CAN负载
			UINT8 bit75:3;
		}str;
	}uni;
	UINT8 MsgType;//消息类型，bit7=0表示请求，bit7=1表示应答
	UINT8 destAddr;//can目标地址
	UINT8 srcAddr;//can源地址			
}MSG_CanHeader,*pMSG_CanHeader;

//--应用层消息头，一个CAN帧 13字节
typedef struct  
{
	UINT8 Flg;//can帧信息，bit7标准帧、扩展帧,bit6数据帧、远程帧,bit3:bit0有效数据长度 为0时做校验
	union{
		UINT8 Pcnt;//can帧计数器 为0时做校验
		struct{
			UINT8 FrmCnt:5;//帧计数器 0到31 32*8=256字节CAN负载
			UINT8 bit75:3;
		}str;
	}uni;
	UINT8 MsgType;//消息类型，bit7=0表示请求，bit7=1表示应答
	UINT8 srcAddr;//can源地址
	UINT8 destAddr;//can目标地址	
	UINT16 length;//消息长度，包括消息头和消息体
	UINT16 serialNo;//消息序号，从0开始顺序增长，到最大值回到0。
	UINT16 replyNo;	//应答序号，只出现在应答消息中，用于匹配请求消息	
	UINT16 crc16;//CRC校验 为0时做校验
}MSG_Header1,*pMSG_Header1;

//---------------------------------------------------------------------------------------
//应用层消息定义
//---------------------------------------------------------------------------------------
//--应用层消息头
typedef struct  
{
	UINT8 Sof;
	UINT8 MsgType;//消息类型，bit7=0表示请求，bit7=1表示应答
	UINT16 srcAddr;//源地址
	UINT16 destAddr;//目标地址
	UINT16 length;//消息长度，包括消息头和消息体
	UINT16 serialNo;//消息序号，从0开始顺序增长，到最大值回到0。
	UINT16 replyNo;	//应答序号，只出现在应答消息中，应答时把请求消息的serialNo填充返回去，
									//用于匹配请求消息	
	UINT16 crc16;//CRC校验 为0时做校验
}MSG_Header,*pMSG_Header;

//--通用应答帧
typedef struct 
{
	MSG_Header header;//消息头				
	UINT16  AckCode;//应答返回码:看AckCode定义	
	UINT8 Eof;
}MSG_ACK,*pMSG_ACK;

//-----------------------------------------------------------------------------
//--注册包，节点到服务器，不带任何信息，下位机程序要通用
//应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头		
	UINT16  protocolVersion;  //协议版本号, 这个版本固定为0
	UINT16 StringLen;
	char  gatewayId[STRING_MAXLEN];   //网关标识，零结束字符串 必须放在结构体最后
	UINT8 Eof;
}MSG_Login,*pMSG_Login;

//--节点主动退出网络，
//应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头				
	UINT16 StringLen;
	char  gatewayId[STRING_MAXLEN];   //网关标识，零结束字符串 
	UINT8 Eof;
}MSG_Logout,*pMSG_Logout;

//--节点心跳
//不需要应答
typedef struct 
{
	MSG_Header header;//消息头	
	UINT8 Eof;
}MSG_Heartbeat,*pMSG_Heartbeat;

//--产量，定时时间为0时，由服务器请求（轮询模式）
//不需要应答
typedef struct 
{
	MSG_Header header;//消息头	
	UINT16  Speed;
	UINT32  Production;//产量
	UINT8 	Eof;
}MSG_Production,*pMSG_Production;

//--班组任务信息，可能在本地被修改
//应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头	
	UINT16  WorkNum;//工号
  UINT32  ProductTask;//生产任务
	UINT8   Class;//班次  
  UINT8 	Eof;
}MSG_TaskInfo,*pMSG_TaskInfo;

//--机器状态，定时时间为0时，由服务器请求（轮询模式）
//应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头	
	UINT8   MacState;//机器状态，1开车、0停车
	UINT8   MacErr;//机器故障信息，0x00无故障，0x01：断纱，0x02。。。。
	UINT32  StopTmLen;//停机时长 单位：秒
  UINT8	  Eof;	
}MSG_MacState,*pMSG_MacState;

//-----------------------------------------------------------------------------
//--机器信息
//--应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头	
	UINT8   Row; //行单条龙头数量
	UINT8   Col;//列龙头条数量
	UINT16   WeftDensity;//纬密 原始数据 x10再传输
	UINT16   OpeningDegree;//开度 原始数据 x10再传输
	UINT8   OutNum;//同时产出条数
	UINT8   Installation;//装造bit76：00左前 01左后 02右前 03右后 ; bbit5：0前后不翻转 1前后翻转 ; bit4；反色 ;bit3：HX
	UINT8   CardSlot;//使用第几列，8列，1有效，0无效
  UINT8 	Eof;	
}MSG_MacInfo,*pMSG_MacInfo;

//--电控版本信息
//--应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头	
	UINT8   McuVer; //MCU软件版本
	UINT8   UiVer;  //界面版本
  UINT8   HwVer;  //硬件版本
	UINT8 	Eof;
}MSG_FirmWareInfo,*pMSG_FirmWareInfo;

//--花样参数，下发文件前先下传此参数，节点收到此消息后，比对文件大小和
//磁盘空间大小，如果超出磁盘空间则返回空间不足错误码
//--应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头	
	UINT16  TotalWeft;//循环总纬数
	UINT16  Warp;//针数 经纱
	UINT16  FileSize;//文件长度 Byte
	UINT16  StringLen;
  char   FileName[STRING_MAXLEN];//生产文件名，最大STRING_MAXLEN字节
	UINT8 Eof;
}MSG_PatPara,*pMSG_PatPara;

//--传送花样文件，服务器到节点，
//--应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头
  UINT16 Totalpackets;//总包数
  UINT16 packetCnt;//当前包序号
  UINT8 Datasize;//当前包有效数据大小
  UINT8  Data[256];//生产文件数据
	UINT8 Eof;
}MSG_PatFile,*pMSG_PatFile;

//--获取机器信息，需应答
//--应答，返回机器配置信息,结构体：MSG_MacInfo
typedef struct 
{
	MSG_Header header;//消息头
	UINT8 Eof;
}MSG_GetMacInfo,*pMSG_GetMacInfo;

//--获取生产任务信息，需应答
//--应答，返回生产任务,结构体：MSG_TaskInfo
typedef struct 
{
	MSG_Header header;//消息头
	UINT8 Eof;
}MSG_GetTaskInfo,*pMSG_GetTaskInfo;

//--获取产量信息，需应答
//--应答,返回产量,结构体：MSG_Production
typedef struct 
{
	MSG_Header header;//消息头
	UINT8 Eof;
}MSG_GetProduction,*pMSG_GetProduction;

//--获取版本信息，需应答
//--应答,返回产量,结构体：MSG_FirmWareInfo
typedef struct 
{
	MSG_Header header;//消息头
	UINT8 Eof;
}MSG_GetFirmWareInfo,*pMSG_GetFirmWareInfo;

//--获取机器状态信息
//--应答,结构体：MSG_MacState
typedef struct 
{
	MSG_Header header;//消息头
	UINT8 Eof;
}MSG_GetMacState,*pMSG_GetMacState;

//--获取生产文件信息
//--应答,结构体：MSG_PatPara
typedef struct 
{
	MSG_Header header;//消息头
	UINT8 Eof;
}MSG_GetPatPara,*pMSG_GetPatPara;

//--设置节点定时间隔
//--应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头
  UINT8   Interval;//单位分钟，为0时等待停止主动上传
	UINT8 	Eof;
}MSG_Interval,*pMSG_Interval;

//--设置节点时间
//--应答，结构体：MSG_ACK。
typedef struct 
{
	MSG_Header header;//消息头
	UINT8 hour;//时
	UINT8 minute;//分
	UINT8 second;//秒
	UINT8 date;//日
	UINT8 month;//月
	UINT16 year;//年  
	UINT8 Eof;
}MSG_SetTime,*pMSG_SetTime;

#pragma pack()    //设置１字节对齐
//////////////////////////////////////////////////////////报文定义结束/////////////////////////////////////////////////////////////////////////

#endif


