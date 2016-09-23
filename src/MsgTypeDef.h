/*************************************************
���ڣ�20160722
��ݣ�Gordon
�޸ļ�¼��
//---------------------------------------------------------------------
20160914 
1�����µ���һ����Ϣ�ṹ�壬���ӻ������͵�
MSG_MacInfo-> char macType[3];//���磺A01 A02 B03 C04     �����Ứ������ͨ�Ứ��������

//---------------------------------------------------------------------
20160920
1�����µ���һ����Ϣ�ṹ�壬1��ע����Ϣ���������Ĺ̶����ã�2����ѯ��Ϣ����Ϊ����

*************************************************/
////////////////////////////////////////////////////////���Ķ��忪ʼ///////////////////////////////////////////////////////////////////////////
/*
1���Զ�����Ϣ���ͣ���Ϊ���������ڵ���ڵ㵽�����������࣬
   �����󷽷�����������ʱ��Ӧ����Ϣ����Msg���λ��1����ʾӦ��֡
2��ͨѶ���̣��ڵ��ϵ�����������ע����Ϣ��ֱ���յ���������ACKΪֹ���յ�ACK��ʾ�ڵ������ɹ�
3���ڵ�ע��󣬷���������ڵ��������������Ϣ
4������������������״̬�ȿɶ�ʱ�ϴ���Ҳ���ɷ�������ѯ��ȡ�������ö�������ʵ������Ч��
*/
#ifndef MSG_TYPE_DEF_H
#define MSG_TYPE_DEF_H
#include "preDef.h"
//node TO server
#define MSG_LOGIN        0X01//ע��
#define MSG_LOGOUT       0X02//ע��
#define MSG_HEARTBEAT    0X03//�ڵ�����
#define MSG_PRODUCTION   0X04//����
#define MSG_TASKINFO     0X05//��������
#define MSG_MACSTATE     0X06//����״̬��Ϣ
#define MSG_PATPARA_REPORT 0X07//�ϱ�������Ϣ
#define MSG_MACINFO_REPORT 0x08//�ϱ�������Ϣ
#define MSG_MAXNODEID    0X3F//�ڵ㵽�����������ϢID 

//server TO node
#define MSG_SETMACINFO        0X40  //���û�����Ϣ �ṹ�壺MSG_MacInfo
#define MSG_SETPATPARA        0X41  //�´�������Ϣ �ṹ�壺MSG_PatPara
#define MSG_FIGUREFILE        0X42  //�ͻ����ļ�   �ṹ�壺MSG_PatFile
#define MSG_SETINTERVAL       0X43  //���ýڵ㶨ʱ��Ϣʱ���� �ṹ�壺MSG_Interval
#define MSG_SETTIME			  		0X44  //���ýڵ�ʱ�� �ṹ�壺MSG_SetTime
//
#define MSG_GETMACSTATE       0X45  //��ȡ����״̬
#define MSG_GETPRODUCTION     0X46  //��ȡ���������Ϣ

//˫��ͨ��
#define MSG_COMACK		  0XF0  //ͨ��ack

//��Ϣ����bit7����
#define MSG_REQUEST (0<<7)
#define MSG_REPLY   (1<<7)

//AckCode����
#define ACK_OK       0x00  //Ӧ��OK
#define ACK_FAIL     0x01  //Ӧ��ʧ��
#define ACK_CRCERR   0x02  //crcУ�����
#define ACK_DATALOSS 0x03  //���ݰ�������
#define ACK_NOT_LOGIN   0X04//û��ע��
#define ACK_MSG_ERROR    0x05  //��Ϣ��ʽ�������ݴ���
#define ACK_NOT_SUPPORT_TYPE    0x06  //Ӧ�����֧��
#define ACK_OUTOFMEM    0x07  //�洢�ռ䲻��
#define ACK_FILEBREAK    0x08  //�ļ�������
#define ACK_TIMEOUT    0x09  		//��ʱ

//֡������Ϣ
#define COM_FRM_HEAD 0xaa//����֡�̶�֡��һ�ֽ�Ϊ0xaa
#define COM_FRM_END 0X55//֡��β�̶�0x55
#define ComFrameLen 16//һ������com֡Ϊ16�ֽ�
#define ExtFrm 0x01//��չ֡
#define StdFrm 0x00//��׼֡
#define NotRmtFrm 0x00//��Զ��֡
#define CanDataSize 8
#define MsgSize 512//
#define HostAddr 0x2002//������ַ��ʼֵ���ɱ�
#define STRING_MAXLEN 20

#pragma pack(1)    //����1�ֽڶ���

//�������ݲ��ô�˸�ʽ����С��ת��
#define Tranverse16(X)                 ((((UINT16)(X) & 0xff00) >> 8) |(((UINT16)(X) & 0x00ff) << 8))
#define Tranverse32(X)                 ((((UINT32)(X) & 0xff000000) >> 24) | \
(((UINT32)(X) & 0x00ff0000) >> 8) | \
(((UINT32)(X) & 0x0000ff00) << 8) | \
(((UINT32)(X) & 0x000000ff) << 24))

//--ͨѶ��ʱ�¼�
#define FIRSTEVENT				0		// ��һ���¼�
#define EVENT_SendMsg			0		// ������Ϣ�ȴ�Ӧ��
#define EVENT_CanFrm		    1		// һ��������can֡
#define EVENT_Packet			2		// Ӧ�ñ��Ľ���������
#define EVENT_Login				3		// ע��
#define EVENT_PRODUCTION		4		// �������
#define EVENT_PARAREPORT		5//�������
#define EVENT_HMI_RECCMD		6	//HMI�������ʱ
#define EVENT_HMI_CHECK			7	//HMI������ѯ
#define EVENT_JAC_RECCMD		8	////JAC ���պ�����ʱ
#define EVENT_JAC_CHECK			9	////JAC ������Ƿ�����
#define LASTEVENT				9		// ���һ���¼�

#define  PRODUCTION_TIMER 10//��
#define  LOGIN_TIMER  3//��


//���ر�ʶ,�ɵ������
#define  GW_ID "HuiGuan-001"

#define ProtocolVer  0

//�붨ʱ����ز���
#define TMR_START 1
#define TMR_STOP  0
#define TMR_OUT   0XFF
typedef struct  
{
	UINT8 TimerFlg;//��ʱ����־0ֹͣ 1���� 0xffʱ�䵽
	UINT16 TimerCnt;//��ʱ��
}TimeEvent,*pTimeEvent;

//--can����ӿ�
typedef struct  
{
	UINT8 Saddr;//canԴ��ַ
	UINT8 Daddr;//canĿ���ַ�����һ�����ӵĵ�ַ
	UINT8 LinkState;//��ǰ����״̬ 0δ���� 1ע��ɹ� 
	UINT8 SendingMsg;//���ڷ��͵���Ϣ����ACK
	UINT8 ReceiveMsgACK;//���յ���ACK
	UINT16 serialNo;//��Ϣ��ţ���0��ʼ˳�������������ֵ�ص�0��
	UINT16 replyNo;	//Ӧ����ţ�ֻ������Ӧ����Ϣ�У�����ƥ��������Ϣ	
}CanNetif,*pCanNetif;

//--ZigBee����ӿ�
typedef struct  
{
	UINT16 IdLen;
	char gwID[STRING_MAXLEN];
	UINT8 chan;             /**< \brief ͨ����        */
  UINT16 panid;         /**< \brief PanID         */
	UINT16 Saddr;//canԴ��ַ
	UINT16 Daddr;//canĿ���ַ�����һ�����ӵĵ�ַ
	UINT8 LinkState;//��ǰ����״̬ 0δ���� 1ע��ɹ� 
	UINT8 SendingMsg;//���ڷ��͵���Ϣ����ACK
	UINT8 ReceiveMsgACK;//���յ���ACK
	UINT16 serialNo;//��Ϣ��ţ���0��ʼ˳�������������ֵ�ص�0��
	UINT16 replyNo;	//Ӧ����ţ�ֻ������Ӧ����Ϣ�У�����ƥ��������Ϣ	
	UINT8   Interval;//�����ϴ����
}ZigNetif,*pZigNetif;

typedef struct  
{	
	UINT8 SendingMsg;//���ڷ��͵���Ϣ����ACK	
	UINT16 serialNo;//��Ϣ��ţ���0��ʼ˳�������������ֵ�ص�0��	
}SendmsgQ,*pSendmsgQ;


//---------------------------------------------------------------------------------------
// can�ײ���Ϣ��ʽ���壬ʹ��canתuartģ��
//---------------------------------------------------------------------------------------
//--����can֡����
typedef struct  
{
	UINT8 Flg;//can֡��Ϣ��bit7��׼֡����չ֡,bit6����֡��Զ��֡,bit3:bit0��Ч���ݳ���
	union{
	UINT8 Pcnt;//can֡��������bit4��bit0��ʾ�ڼ�֡
	struct{
		UINT8 FrmCnt:5;//֡������		
		UINT8 bit75:3;
	}str;
	}uni;
	UINT8 Msg;//��Ϣ���ͣ�bit7=0��ʾ����bit7=1��ʾӦ��
	UINT8 Daddr;//canĿ���ַ
	UINT8 Saddr;//canԴ��ַ
	UINT8 Data[8];//����
}CanFrame,*pCanFrame;

//--��������֡��ʽ
typedef struct  
{
	UINT8 Head;//�̶�Ϊ0xAA
	UINT8 StdExtFlg;//��׼֡����չ֡��
	UINT8 DatRmtFlg;//����֡��Զ��֡
	UINT8 DatLen;//��Ч���ݳ��� max=8
	union{
		UINT8 Pcnt;//can֡��������bit4��bit0��ʾ�ڼ�֡����0��31,��������32*8=256byte
		struct{
			UINT8 FrmCnt:5;			
			UINT8 bit75:3;//֡������
		}str;
	}uni0;
	UINT8 Msg;//��Ϣ���ͣ�bit7=0��ʾ����bit7=1��ʾӦ��
	UINT8 Daddr;//canĿ���ַ
	UINT8 Saddr;//canԴ��ַ
	union{
		UINT8 Data[8];//����
		struct  
		{			
			UINT16 length;//��Ϣ���ȣ�������Ϣͷ����Ϣ��
			UINT16 serialNo;//��Ϣ��ţ���0��ʼ˳�������������ֵ�ص�0��
			UINT16 replyNo;	//Ӧ����ţ�ֻ������Ӧ����Ϣ�У�����ƥ��������Ϣ	
			UINT16 crc16;//CRCУ�� Ϊ0ʱ��У��
		}str;	
	}uni1;	
}ComFrame,*pComFrame;

//--CAN֡ͷ
typedef struct  
{
	UINT8 Flg;//can֡��Ϣ��bit7��׼֡����չ֡,bit6����֡��Զ��֡,bit3:bit0��Ч���ݳ��� Ϊ0ʱ��У��
	union{
		UINT8 Pcnt;//can֡������ Ϊ0ʱ��У��
		struct{
			UINT8 FrmCnt:5;//֡������ 0��31 32*8=256�ֽ�CAN����
			UINT8 bit75:3;
		}str;
	}uni;
	UINT8 MsgType;//��Ϣ���ͣ�bit7=0��ʾ����bit7=1��ʾӦ��
	UINT8 destAddr;//canĿ���ַ
	UINT8 srcAddr;//canԴ��ַ			
}MSG_CanHeader,*pMSG_CanHeader;

//--Ӧ�ò���Ϣͷ��һ��CAN֡ 13�ֽ�
typedef struct  
{
	UINT8 Flg;//can֡��Ϣ��bit7��׼֡����չ֡,bit6����֡��Զ��֡,bit3:bit0��Ч���ݳ��� Ϊ0ʱ��У��
	union{
		UINT8 Pcnt;//can֡������ Ϊ0ʱ��У��
		struct{
			UINT8 FrmCnt:5;//֡������ 0��31 32*8=256�ֽ�CAN����
			UINT8 bit75:3;
		}str;
	}uni;
	UINT8 MsgType;//��Ϣ���ͣ�bit7=0��ʾ����bit7=1��ʾӦ��
	UINT8 srcAddr;//canԴ��ַ
	UINT8 destAddr;//canĿ���ַ	
	UINT16 length;//��Ϣ���ȣ�������Ϣͷ����Ϣ��
	UINT16 serialNo;//��Ϣ��ţ���0��ʼ˳�������������ֵ�ص�0��
	UINT16 replyNo;	//Ӧ����ţ�ֻ������Ӧ����Ϣ�У�����ƥ��������Ϣ	
	UINT16 crc16;//CRCУ�� Ϊ0ʱ��У��
}MSG_Header1,*pMSG_Header1;

//---------------------------------------------------------------------------------------
//Ӧ�ò���Ϣ����
//---------------------------------------------------------------------------------------
//--Ӧ�ò���Ϣͷ
typedef struct  
{
	UINT8 Sof;
	UINT8 MsgType;//��Ϣ���ͣ�bit7=0��ʾ����bit7=1��ʾӦ��
	UINT16 srcAddr;//Դ��ַ
	UINT16 destAddr;//Ŀ���ַ
	UINT16 length;//��Ϣ���ȣ�������Ϣͷ����Ϣ��
	UINT16 serialNo;//��Ϣ��ţ���0��ʼ˳�������������ֵ�ص�0��
	UINT16 replyNo;	//Ӧ����ţ�ֻ������Ӧ����Ϣ�У�Ӧ��ʱ��������Ϣ��serialNo��䷵��ȥ��
									//����ƥ��������Ϣ	
	UINT16 crc16;//CRCУ�� Ϊ0ʱ��У��
}MSG_Header,*pMSG_Header;

//--ͨ��Ӧ��֡
typedef struct 
{
	MSG_Header header;//��Ϣͷ				
	UINT16  AckCode;//Ӧ�𷵻���:��AckCode����	
	UINT8 Eof;
}MSG_ACK,*pMSG_ACK;

//-----------------------------------------------------------------------------
//--ע������ڵ㵽����������������������Ϣ
typedef struct 
{
	MSG_Header header;//��Ϣͷ		
	UINT16  protocolVersion;  //Э��汾��, ����汾�̶�Ϊ0
	UINT16 StringLen;
	char  gatewayId[STRING_MAXLEN];   //���ر�ʶ��������ַ��� ������ڽṹ�����
	UINT8   Row; //�е�����ͷ����
	UINT8   Col;//����ͷ������	
	UINT16  Warp;//���� ��ɴ Row*Col*8	
	UINT8   Installation;//װ��bbit76��00��ǰ 01��� 02��ǰ 03�Һ� ; bbit5��0ǰ�󲻷�ת 1ǰ��ת ; bit4����ɫ ;bit3��HX
	UINT8   CardSlot;//ʹ�õڼ��У�8�У�1��Ч��0��Ч
	char    macID[4];//��̨��� �磺A01 A02 B20
  char    macType[4];//�������� �磺A01 A02 B03 C04     �����Ứ������ͨ�Ứ��������
	
	UINT8   McuVer; //MCU����汾
	UINT8   UiVer;  //����汾
  UINT8   Hw1Ver;  //Ӳ��1�汾
	UINT8   Hw2Ver;  //Ӳ��2�汾
	
	UINT8 	Eof;
}MSG_Login,*pMSG_Login;

//--����״̬���ɷ�����������ѯģʽ ���1���ӣ�
typedef struct 
{
	MSG_Header header;//��Ϣͷ	
	UINT16  Speed;//��ǰ����
	UINT8   MacState;//����״̬��1������0ͣ��
	UINT8   MacErr;//����������Ϣ��0x00�޹��ϣ�0x01����ɴ��0x02��������
	UINT32  IdlTmLen;//ͣ��ʱ�� ��λ����
  UINT8	  Eof;	
}MSG_MacState,*pMSG_MacState;

//--������ز������ɷ�����������ѯģʽ��
typedef struct 
{
	MSG_Header header;//��Ϣͷ	

	UINT32  RunTmLen;//����ʱ�� ��λ����
	//�Ƽ�ʹ��
	char   	Class;//��� ���确A�� ��B�� ��C�� 	
	char  	WorkNum[6];//���� ���� ��A1234��	
	UINT32  ClassTmLen;//��ǰ���ſ���ʱ��
	UINT32  ClassOut;//��ǰ���Ų���
	//�������
  UINT32  PatTask;//������������ ��
	UINT32  TotalOut;//����ɵ��ܲ���	
	UINT32  RemainTm;//Ԥ������ʣ�����ʱ��	
	UINT8   OutNum;//ͬʱ��������	
	UINT16  WeftDensity;//γ�� ԭʼ���� x100�ٴ���
	UINT16  OpeningDegree;//���� ԭʼ���� x100�ٴ���
	UINT16  TotalWeft;//ѭ����γ��
	UINT16  StringLen;//�ļ�������
	char   	FileName[STRING_MAXLEN];//�����ļ��������STRING_MAXLEN�ֽ�
	
	UINT8 	Eof;
}MSG_Production,*pMSG_Production;

//--�ڵ������˳����磬
//Ӧ�𣬽ṹ�壺MSG_ACK��
typedef struct 
{
	MSG_Header header;//��Ϣͷ				
	UINT16 StringLen;
	char  gatewayId[STRING_MAXLEN];   //���ر�ʶ��������ַ��� 
	UINT8 Eof;
}MSG_Logout,*pMSG_Logout;

//--�����������·��ļ�ǰ���´��˲������ڵ��յ�����Ϣ�󣬱ȶ��ļ���С��
//���̿ռ��С������������̿ռ��򷵻ؿռ䲻�������
//--Ӧ�𣬽ṹ�壺MSG_ACK��
typedef struct 
{
	MSG_Header header;//��Ϣͷ		
	UINT16  Warp;//���� ��ɴ Row*Col*8
	UINT16  FileSize;//�ļ����� Byte
	UINT16  StringLen;
  char   FileName[STRING_MAXLEN];//�����ļ��������STRING_MAXLEN�ֽ�
	UINT8 Eof;
}MSG_PatPara,*pMSG_PatPara;

//--���ͻ����ļ������������ڵ㣬
//--Ӧ�𣬽ṹ�壺MSG_ACK��
typedef struct 
{
	MSG_Header header;//��Ϣͷ
  UINT16 Totalpackets;//�ܰ���
  UINT16 packetCnt;//��ǰ�����
  UINT8 Datasize;//��ǰ����Ч���ݴ�С
  UINT8  Data[256];//�����ļ�����
	UINT8 Eof;
}MSG_PatFile,*pMSG_PatFile;


//--��ȡ���������Ϣ����Ӧ��
//--Ӧ��,���ز���,�ṹ�壺MSG_Production
typedef struct 
{
	MSG_Header header;//��Ϣͷ
	UINT8 Eof;
}MSG_GetProduction,*pMSG_GetProduction;

//--��ȡ����״̬��Ϣ
//--Ӧ��,�ṹ�壺MSG_MacState
typedef struct 
{
	MSG_Header header;//��Ϣͷ
	UINT8 Eof;
}MSG_GetMacState,*pMSG_GetMacState;

//--���ýڵ㶨ʱ���
//--Ӧ�𣬽ṹ�壺MSG_ACK��
typedef struct 
{
	MSG_Header header;//��Ϣͷ
  UINT8   Interval;//��λ���ӣ�Ϊ0ʱ�ȴ�ֹͣ�����ϴ�
	UINT8 	Eof;
}MSG_Interval,*pMSG_Interval;

//--���ýڵ�ʱ��
//--Ӧ�𣬽ṹ�壺MSG_ACK��
typedef struct 
{
	MSG_Header header;//��Ϣͷ
	UINT8 hour;//ʱ
	UINT8 minute;//��
	UINT8 second;//��
	UINT8 date;//��
	UINT8 month;//��
	UINT16 year;//��  
	UINT8 Eof;
}MSG_SetTime,*pMSG_SetTime;

#pragma pack()    //���ã��ֽڶ���
//////////////////////////////////////////////////////////���Ķ������/////////////////////////////////////////////////////////////////////////

#endif

