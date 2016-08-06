#include <muduo/net/TcpServer.h>

#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <utility>

#include <set>
#include <stdio.h>
#include <unistd.h>
#include <sstream>

#include "MsgTypeDef.h"
#include "tools.h"
#include "gateway.h"

using namespace muduo;
using namespace muduo::net;

class JacServer
{
 public:
  JacServer(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr, "JacServer")
  {
    server_.setConnectionCallback(
        boost::bind(&JacServer::onConnection, this, _1));
    server_.setMessageCallback(
        boost::bind(&JacServer::onMessage, this, _1, _2, _3));

    loop_->runAfter(2,boost::bind(&JacServer::onTimer,this));

    m_curMsgSerialNo = 0;
    m_iSendNo = 0;
  }

  void start()
  {
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn);

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

  void onTimer();

  void sendAll(Buffer* buf);

  void sendReplyAck(TcpConnection* conn, pMSG_Header srcheader,UINT8 ACK_code);

  UINT16 getMsgSerialNo();

  UINT8  getSendCmd();


  typedef std::set<TcpConnectionPtr> ConnectionList;
  EventLoop* loop_;
  TcpServer server_;
  ConnectionList connections_;

  UINT16 m_curMsgSerialNo;
  UINT16 m_localAddr;

  //临时存放 目标地址
  UINT16 m_destAddr;
  UINT16 m_iSendNo;

  // tmp
  // std::vector<UINT16> m_vNodes;
  // std::map<std::string,gateway> m_mGateways;
  gateway* m_curGateway;

};

UINT8 JacServer::getSendCmd()
{
  // m_iSendNo++;

  UINT16 iTmp = m_iSendNo % 11;

  if (iTmp == 0)
  {
    return MSG_SETMACINFO;
  }
  else if(iTmp == 1)
  {
    return MSG_SETPATPARA;
  }
  else if(iTmp == 2)
  {
    return MSG_FIGUREFILE;
  }
  else if(iTmp == 3)
  {
    return MSG_GETMACINFO;
  }
  else if(iTmp == 4)
  {
    return MSG_GETTASKINFO; 
  }
  else if(iTmp == 5)
  {
    return MSG_GETPRODUCTION; 
  }
  else if(iTmp == 6)
  {
    return MSG_GETFIRMWAREINFO;
  }
  else if(iTmp == 7)
  {
    return MSG_SETINTERVAL;
  }
  else if(iTmp == 8)
  {
    return MSG_SETTIME;
  }
  else if(iTmp == 9)
  {
    return MSG_GETMACSTATE;
  }
  else if(iTmp == 10)
  {
    return MSG_GETPATPARA;
  }
}

void JacServer::onConnection(const TcpConnectionPtr& conn)
{
  LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
            << conn->localAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

  // if (conn->connected())
  //   {
  //     connections_.insert(conn);
  //   }
  //   else
  //   {
  //     connections_.erase(conn);
  //   }

}

void JacServer::onTimer()
{
  Buffer tBuf;
  UINT16 msgLen = 0;

  if (m_curGateway == NULL)
  {
    return;
  }

  //循环设置已经注册的节点
  if (m_curGateway->getNodeSize() == 0 )
  {
    LOG_INFO << "no node registed now!";
    loop_->runAfter(5, boost::bind(&JacServer::onTimer, this));
    return;
  }
  else
  {
    msgLen = sizeof(ModifyGateWayDestAddr);
    ModifyGateWayDestAddr* stuModifyGateWayDestAddr = (ModifyGateWayDestAddr*)new char(msgLen);
    stuModifyGateWayDestAddr->protocolTag1 = 0xDE;
    stuModifyGateWayDestAddr->protocolTag2 = 0xDF;
    stuModifyGateWayDestAddr->protocolTag3 = 0xEF;
    stuModifyGateWayDestAddr->funcCode = 0xD2;
    stuModifyGateWayDestAddr->addr = m_curGateway->getCurNode()->addr;

    tBuf.append(stuModifyGateWayDestAddr,msgLen);

    sendAll(&tBuf);   // need modify

  }


  ////////////////////////////////////////

  UINT16 tmpCrc=0;
  UINT8  tmpCmd = getSendCmd();
  UINT16 tmpNo = getMsgSerialNo();
  LOG_INFO << "onTimer: cmd＝"<< tmpCmd;
  LOG_INFO << "onTimer: tmpNo=" <<tmpNo;

  if(tmpCmd == MSG_GETPRODUCTION )
  {
    msgLen = sizeof(MSG_GetProduction);
    MSG_GetProduction* stuGetProduction = (MSG_GetProduction*)new char[msgLen];

    stuGetProduction->header.Sof=COM_FRM_HEAD;
    stuGetProduction->header.MsgType = MSG_GETPRODUCTION;
    stuGetProduction->header.srcAddr = (m_localAddr);
    stuGetProduction->header.destAddr = Tranverse16(m_destAddr);
    stuGetProduction->header.length = Tranverse16(msgLen) ;
    stuGetProduction->header.serialNo = Tranverse16(tmpNo);
    stuGetProduction->header.replyNo = 0;
    stuGetProduction->header.crc16 = 0;
    stuGetProduction->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuGetProduction->header.Sof,msgLen);
    stuGetProduction->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuGetProduction, msgLen);
  }
  else if( tmpCmd == MSG_SETMACINFO)
  {
    //#define MSG_SETMACINFO           0X40  //设置机器信息
    msgLen = sizeof(MSG_MacInfo);
    MSG_MacInfo* stuSetMacInfo = (MSG_MacInfo*)new char[sizeof(MSG_MacInfo)];

    stuSetMacInfo->header.Sof=COM_FRM_HEAD;
    stuSetMacInfo->header.MsgType = MSG_SETMACINFO;
    stuSetMacInfo->header.srcAddr = (m_localAddr);
    stuSetMacInfo->header.destAddr = Tranverse16(m_destAddr);
    stuSetMacInfo->header.length = Tranverse16(msgLen) ;
    stuSetMacInfo->header.serialNo = Tranverse16(tmpNo);
    stuSetMacInfo->header.replyNo = 0;
    stuSetMacInfo->header.crc16 = 0;

    stuSetMacInfo->Row = 2;
    stuSetMacInfo->Col = 3;
    stuSetMacInfo->WeftDensity = Tranverse16(0x01);
    stuSetMacInfo->OpeningDegree = Tranverse16(0x01);
    stuSetMacInfo->OutNum = 5;
    stuSetMacInfo->Installation = 01;
    stuSetMacInfo->CardSlot = 1;
    stuSetMacInfo->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuSetMacInfo->header.Sof,msgLen);
    stuSetMacInfo->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuSetMacInfo, msgLen);
  }
  else if( tmpCmd == MSG_SETPATPARA)
  {
    // #define MSG_SETPATPARA           0X41  //下传花样信息
    msgLen = sizeof(MSG_PatPara);
    MSG_PatPara* stuSetPatPara = (MSG_PatPara*)new char[sizeof(MSG_PatPara)];

    stuSetPatPara->header.Sof=COM_FRM_HEAD;
    stuSetPatPara->header.MsgType = MSG_SETPATPARA;
    stuSetPatPara->header.srcAddr = (m_localAddr);
    stuSetPatPara->header.destAddr = Tranverse16(m_destAddr);
    stuSetPatPara->header.length = Tranverse16(msgLen) ;
    stuSetPatPara->header.serialNo = Tranverse16(tmpNo);
    stuSetPatPara->header.replyNo = 0;
    stuSetPatPara->header.crc16 = 0;

    stuSetPatPara->TotalWeft = Tranverse16(24);
    stuSetPatPara->Warp = Tranverse16(6);
    stuSetPatPara->FileSize = Tranverse16(66);
    stuSetPatPara->StringLen = Tranverse16(24);

    string tmpStr = "test.file";
    strcpy(stuSetPatPara->FileName, tmpStr.c_str());

    stuSetPatPara->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuSetPatPara->header.Sof,msgLen);
    stuSetPatPara->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuSetPatPara, msgLen);
  }
  else if (tmpCmd == MSG_FIGUREFILE)
  {
    // #define MSG_FIGUREFILE        0X42  //送花样文件
    msgLen = sizeof(MSG_PatFile);
    MSG_PatFile* stuSetPatFile = (MSG_PatFile*)new char[sizeof(MSG_PatFile)];

    stuSetPatFile->header.Sof=COM_FRM_HEAD;
    stuSetPatFile->header.MsgType = MSG_FIGUREFILE;
    stuSetPatFile->header.srcAddr = m_localAddr;
    stuSetPatFile->header.destAddr = Tranverse16(m_destAddr);
    stuSetPatFile->header.length = Tranverse16(msgLen) ;
    stuSetPatFile->header.serialNo = Tranverse16(tmpNo);
    stuSetPatFile->header.replyNo = 0;
    stuSetPatFile->header.crc16 = 0;

    stuSetPatFile->Totalpackets = Tranverse16(100);
    stuSetPatFile->packetCnt = Tranverse16(33);
    stuSetPatFile->Datasize = 35;
    // stuSetPatFile->Data = {0};
    memset(stuSetPatFile->Data,0,sizeof(UINT8)*256);

    stuSetPatFile->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuSetPatFile->header.Sof,msgLen);
    stuSetPatFile->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuSetPatFile, msgLen);
  }
  else if (tmpCmd == MSG_GETMACINFO)
  {
      // #define MSG_GETMACINFO        0X43  //获取机器信息
    msgLen = sizeof(MSG_GetMacInfo);
    MSG_GetMacInfo* stuGetMacInfo = (MSG_GetMacInfo*)new char[sizeof(MSG_GetMacInfo)];

    stuGetMacInfo->header.Sof=COM_FRM_HEAD;
    stuGetMacInfo->header.MsgType = MSG_GETMACINFO;
    stuGetMacInfo->header.srcAddr = (m_localAddr);
    stuGetMacInfo->header.destAddr = Tranverse16(m_destAddr);
    stuGetMacInfo->header.length = Tranverse16(msgLen) ;
    stuGetMacInfo->header.serialNo = Tranverse16(tmpNo);
    stuGetMacInfo->header.replyNo = 0;
    stuGetMacInfo->header.crc16 = 0;
    stuGetMacInfo->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuGetMacInfo->header.Sof,msgLen);
    stuGetMacInfo->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuGetMacInfo, msgLen);
  }
  else if( tmpCmd == MSG_GETTASKINFO)
  {
      // #define MSG_GETTASKINFO       0X44  //获取生产任务

    msgLen = sizeof(MSG_GetTaskInfo);
    MSG_GetTaskInfo* stuGetTaskInfo = (MSG_GetTaskInfo*)new char[sizeof(MSG_GetTaskInfo)];

    stuGetTaskInfo->header.Sof=COM_FRM_HEAD;
    stuGetTaskInfo->header.MsgType = MSG_GETTASKINFO;
    stuGetTaskInfo->header.srcAddr = (m_localAddr);
    stuGetTaskInfo->header.destAddr = Tranverse16(m_destAddr);
    stuGetTaskInfo->header.length = Tranverse16(msgLen) ;
    stuGetTaskInfo->header.serialNo = Tranverse16(tmpNo);
    stuGetTaskInfo->header.replyNo = 0;
    stuGetTaskInfo->header.crc16 = 0;
    stuGetTaskInfo->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuGetTaskInfo->header.Sof,msgLen);
    stuGetTaskInfo->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuGetTaskInfo, msgLen);
  }

  else if(tmpCmd == MSG_GETFIRMWAREINFO)
  {
      // #define MSG_GETFIRMWAREINFO   0X46  //获取固件信息等
    msgLen = sizeof(MSG_GetFirmWareInfo);
    MSG_GetFirmWareInfo* stuGetFirmWareInfo = (MSG_GetFirmWareInfo*)new char[sizeof(MSG_GetFirmWareInfo)];

    stuGetFirmWareInfo->header.Sof=COM_FRM_HEAD;
    stuGetFirmWareInfo->header.MsgType = MSG_GETFIRMWAREINFO;
    stuGetFirmWareInfo->header.srcAddr = (m_localAddr);
    stuGetFirmWareInfo->header.destAddr = Tranverse16(m_destAddr);
    stuGetFirmWareInfo->header.length = Tranverse16(msgLen);
    stuGetFirmWareInfo->header.serialNo = Tranverse16(tmpNo);
    stuGetFirmWareInfo->header.replyNo = 0;
    stuGetFirmWareInfo->header.crc16 = 0;
    stuGetFirmWareInfo->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuGetFirmWareInfo->header.Sof,msgLen);
    stuGetFirmWareInfo->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuGetFirmWareInfo, msgLen);
  }
  else if (tmpCmd == MSG_SETINTERVAL)
  {
// #define MSG_SETINTERVAL       0X47  //设置节点定时消息时间间隔

    msgLen = sizeof(MSG_Interval);
    MSG_Interval* stuInterval = (MSG_Interval*)new char[sizeof(MSG_Interval)];

    stuInterval->header.Sof=COM_FRM_HEAD;
    stuInterval->header.MsgType = MSG_SETINTERVAL;
    stuInterval->header.srcAddr = (m_localAddr);
    stuInterval->header.destAddr = Tranverse16(m_destAddr);
    stuInterval->header.length = Tranverse16(msgLen) ;
    stuInterval->header.serialNo = Tranverse16(tmpNo);
    stuInterval->header.replyNo = 0;
    stuInterval->header.crc16 = 0;
    stuInterval->Interval = 5;    // set
    stuInterval->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuInterval->header.Sof,msgLen);
    stuInterval->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuInterval, msgLen);
  }
  else if(tmpCmd == MSG_SETTIME)
  {
      // #define MSG_SETTIME           0X48  //设置节点时间
    msgLen = sizeof(MSG_SetTime);
    MSG_SetTime* stuSetTime = (MSG_SetTime*)new char[sizeof(MSG_SetTime)];

    stuSetTime->header.Sof=COM_FRM_HEAD;
    stuSetTime->header.MsgType = MSG_SETTIME;
    stuSetTime->header.srcAddr = (m_localAddr);
    stuSetTime->header.destAddr = Tranverse16(m_destAddr);
    stuSetTime->header.length = Tranverse16(msgLen);
    stuSetTime->header.serialNo = Tranverse16(tmpNo);
    stuSetTime->header.replyNo = 0;
    stuSetTime->header.crc16 = 0;
    // get system time and set it 
    stuSetTime->hour = 12;
    stuSetTime->minute = 22;
    stuSetTime->second = 30;
    stuSetTime->date = 20;
    stuSetTime->month = 10;
    stuSetTime->year = Tranverse16(2016);

    stuSetTime->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuSetTime->header.Sof,msgLen);
    stuSetTime->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuSetTime, msgLen);
  }
  else if(tmpCmd == MSG_GETMACSTATE)
  {
    // #define MSG_GETMACSTATE       0X49  //获取机器状态信息
    msgLen = sizeof(MSG_GetMacState);
    MSG_GetMacState* stuGetMacState = (MSG_GetMacState*)new char[sizeof(MSG_GetMacState)];

    stuGetMacState->header.Sof=COM_FRM_HEAD;
    stuGetMacState->header.MsgType = MSG_GETMACSTATE;
    stuGetMacState->header.srcAddr = (m_localAddr);
    stuGetMacState->header.destAddr = Tranverse16(m_destAddr);
    stuGetMacState->header.length = Tranverse16(msgLen);
    stuGetMacState->header.serialNo = Tranverse16(tmpNo);
    stuGetMacState->header.replyNo = 0;
    stuGetMacState->header.crc16 = 0;
    stuGetMacState->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuGetMacState->header.Sof,msgLen);
    stuGetMacState->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuGetMacState, msgLen);
  }
  else if (tmpCmd == MSG_GETPATPARA)
  {
      // #define MSG_GETPATPARA        0X4A  //获取花样信息
    msgLen = sizeof(MSG_GetPatPara);
    MSG_GetPatPara* stuGetPatPara = (MSG_GetPatPara*)new char[sizeof(MSG_GetPatPara)];

    stuGetPatPara->header.Sof=COM_FRM_HEAD;
    stuGetPatPara->header.MsgType = MSG_GETPATPARA;
    stuGetPatPara->header.srcAddr = (m_localAddr);
    stuGetPatPara->header.destAddr = Tranverse16(m_destAddr);
    stuGetPatPara->header.length = Tranverse16(msgLen);
    stuGetPatPara->header.serialNo = Tranverse16(tmpNo);
    stuGetPatPara->header.replyNo = 0;
    stuGetPatPara->header.crc16 = 0;
    stuGetPatPara->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuGetPatPara->header.Sof,msgLen);
    stuGetPatPara->header.crc16 = Tranverse16(tmpCrc);

    tBuf.append(stuGetPatPara, msgLen);
  }

  sendAll(&tBuf);

  loop_->runAfter(3, boost::bind(&JacServer::onTimer, this));

}

void JacServer::sendAll(Buffer* buf)
{
  for (ConnectionList::iterator it = connections_.begin();
        it != connections_.end();
        ++it)
    {
      get_pointer(*it)->send(buf);
    }

     m_iSendNo++;
}

void JacServer::sendReplyAck(TcpConnection* conn, pMSG_Header srcheader,UINT8 ACK_code)
{
  MSG_ACK stuAck;
  pMSG_Header pheader = &stuAck.header;       
  UINT16 msgLen = sizeof(MSG_ACK);
  UINT16 tmpCrc=0;

  // ack
  Buffer ackBuf;
  // MSG_ACK* stuAck = (MSG_ACK*)new char[sizeof(MSG_ACK)];
  // LOG_INFO <<"haahhahhahahahhahhhhahhhhhhhh";
  UINT16 tmpNo = getMsgSerialNo();
  // LOG_INFO << "############################";

  pheader->Sof=COM_FRM_HEAD;
  pheader->MsgType = MSG_COMACK;
  pheader->srcAddr = srcheader->destAddr;
  pheader->destAddr = srcheader->srcAddr;
  pheader->length = Tranverse16(msgLen);
  pheader->serialNo = Tranverse16(tmpNo);
  pheader->replyNo = srcheader->serialNo;
  LOG_INFO << "----------serialNo: " << Tranverse16(pheader->serialNo);
  LOG_INFO <<"---------replyNo: " << Tranverse16(pheader->replyNo);

  pheader->crc16 = 0;

  stuAck.AckCode=Tranverse16(ACK_code);
  stuAck.Eof = COM_FRM_END;

  tmpCrc = CalcCRC16(0,&stuAck.header.Sof,msgLen);
  pheader->crc16=Tranverse16(tmpCrc);


  ackBuf.append(&stuAck, sizeof(MSG_ACK));

  LOG_INFO <<"ack was send!";
  conn->send(&ackBuf);
}

// process data/protocol
void JacServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
  
  LOG_INFO << "readableBytes length = " << buf->readableBytes();

   MSG_Header* tHeader = (MSG_Header*) new char[sizeof(MSG_Header)];
   UINT8 tmpAckCode;

   if (buf->readableBytes() >= sizeof(RspAck))
   {
     pRspAck tmpAck = (pRspAck) new char[sizeof(RspAck)];
     if ((tmpAck->protocolTag1 == 0xDE)
      && (tmpAck->protocolTag2 == 0xDF)
    && (tmpAck->protocolTag3 == 0xEF)
    && (tmpAck->funcCode == 0xD2 ))
     {
       //处理修改目标节点命令反馈
      if (tmpAck->ackCode == 0x00)
      {
        LOG_INFO << "---------modify dest node success!-------";
        m_curGateway->setCurOperatorType(SEND_MESSAGE);
        loop_->runAfter(5, boost::bind(&JacServer::onTimer, this));

      }
      else
      {
        LOG_INFO << "---------modify dest node failed!-------";
        loop_->runAfter(5, boost::bind(&JacServer::onTimer, this));
      }

     }
     
     buf->retrieve(sizeof(RspAck));
     return;
   }

   if (buf->readableBytes() >= sizeof(MSG_Header))
   {
     /* code */
    tHeader = (MSG_Header*)const_cast<char*>(buf->peek());

    LOG_INFO << "MsgType: " << tHeader->MsgType;
    //cout << "msgtype:" <<hex<<tHeader<<endl;
  
    if (tHeader->MsgType == MSG_LOGIN)
    {
      if(buf->readableBytes() < sizeof(MSG_Login))
      {
        sendReplyAck(get_pointer(conn),tHeader,ACK_DATALOSS);
        buf->retrieve(buf->readableBytes());
        return;
      }

       // MSG_Login
       MSG_Login* stuBody = (MSG_Login*)const_cast<char*>(buf->peek());

       if (m_localAddr == 0)
       {
         m_localAddr = (stuBody->header.destAddr);
       }

       if (m_destAddr == 0)
       {
         m_destAddr = Tranverse16(stuBody->header.srcAddr);
       }

       LOG_INFO << "-------------------srcAddr: " << Tranverse16(stuBody->header.srcAddr);

       LOG_INFO << "destAddr: " << Tranverse16(stuBody->header.destAddr);
       LOG_INFO << "length: " << Tranverse16(stuBody->header.length);
       LOG_INFO << "serialNo: " << Tranverse16(stuBody->header.serialNo);
       LOG_INFO << "replyNo: " << Tranverse16(stuBody->header.replyNo);
       LOG_INFO << "crc16: " << stuBody->header.crc16;

       LOG_INFO << "protocolVersion: " << Tranverse16(stuBody->protocolVersion);
       // LOG_INFO << " StringLen: " << Tranverse16(stuBody->StringLen);
       LOG_INFO << "gatewayId: " << stuBody->gatewayId;

       //报文合法性校验
       if(Tranverse16(stuBody->header.length) != sizeof(MSG_Login))
       {
          tmpAckCode = ACK_OUTOFMEM;
          LOG_INFO << "ACK_OUTOFMEM";
       }

       if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
       {
         tmpAckCode = ACK_DATALOSS;
         LOG_INFO << "ACK_DATALOSS";
       }

       if (stuBody->header.destAddr != m_localAddr)
       {
         tmpAckCode = ACK_MSG_ERROR;
         LOG_INFO << "ACK_MSG_ERROR";
       }
       
       // 节点注册成功
       if (tmpAckCode == ACK_OK)
       {
         if (m_curGateway == NULL)
         {
           m_curGateway = new gateway();
         }
         m_curGateway->setName(stuBody->gatewayId);

         pINFO_Node tmpNode = new INFO_Node();      // when to free pointer?
         tmpNode->addr = (stuBody->header.srcAddr);

         m_curGateway->insertNode(tmpNode);

       }

       buf->retrieve(sizeof(MSG_Login));       

        connections_.insert(conn);
       sendReplyAck(get_pointer(conn),&stuBody->header,tmpAckCode);
    }
    // else if (/* condition */) // process the rsp ack ,modify the current node addr
    // {
    //   /* code */
    // }
    else if (tHeader->MsgType == MSG_LOGOUT)
    {
      if(buf->readableBytes() < sizeof(MSG_Logout))
      {
        sendReplyAck(get_pointer(conn),tHeader,ACK_DATALOSS);
        buf->retrieve(buf->readableBytes());
        return;
      }
       //MSG_Logout* stuBody = (MSG_Logout*) new char[sizeof(MSG_Logout)];
       MSG_Logout* stuBody = (MSG_Logout*)const_cast<char*>(buf->peek());


       LOG_INFO << "cmd type: " << stuBody->header.MsgType;

        //报文合法性校验
       if(Tranverse16(stuBody->header.length) != sizeof(MSG_Logout))
       {
          tmpAckCode = ACK_OUTOFMEM;
       }

       if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
       {
         tmpAckCode = ACK_DATALOSS;
       }

       if (stuBody->header.destAddr != m_localAddr)
       {
         tmpAckCode = ACK_MSG_ERROR;
       }

       buf->retrieve(sizeof(MSG_Logout));

       // ack
       // connections_.erase(conn);
       sendReplyAck(get_pointer(conn),&stuBody->header,tmpAckCode);
       
    }
    else if (tHeader->MsgType == MSG_COMACK)
    {
      //common ack
      // MSG_ACK
      if(buf->readableBytes() < sizeof(MSG_ACK))
      {
        // sendReplyAck(get_pointer(conn),tHeader,ACK_DATALOSS);
        LOG_WARN << "data loss in common ack" ;
        buf->retrieve(buf->readableBytes());
        return;
      }
       //MSG_Logout* stuBody = (MSG_Logout*) new char[sizeof(MSG_Logout)];
       MSG_ACK* stuBody = (MSG_ACK*)const_cast<char*>(buf->peek());

       LOG_DEBUG << "common ack,serialNo = " << stuBody->header.serialNo << " | " 
                << "replyNo = " << stuBody->header.replyNo;

        //报文合法性校验
       if(Tranverse16(stuBody->header.length) != sizeof(MSG_Logout))
       {
          tmpAckCode = ACK_OUTOFMEM;
       }

       if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
       {
         tmpAckCode = ACK_DATALOSS;
       }

       if (stuBody->header.destAddr != m_localAddr)
       {
         tmpAckCode = ACK_MSG_ERROR;
       }

       LOG_WARN << "common ack, errCode = " << tmpAckCode;

       buf->retrieve(sizeof(MSG_ACK));
    }
    else if (tHeader->MsgType == (MSG_REPLY|MSG_GETPRODUCTION))
    {
       // get Production

      if(buf->readableBytes() < sizeof(MSG_Production))
      {
        buf->retrieve(buf->readableBytes());
        LOG_INFO << "-------- ACK_DATALOSS ";
        return;
      }

       MSG_Production* stuBody = (MSG_Production*)const_cast<char*>(buf->peek());

       //报文合法性校验
       if(Tranverse16(stuBody->header.length) != sizeof(MSG_Production))
       {
          tmpAckCode = ACK_OUTOFMEM;
       }
       else if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
       {
         tmpAckCode = ACK_DATALOSS;
       }
       else if (stuBody->header.destAddr != m_localAddr)
       {
         tmpAckCode = ACK_MSG_ERROR;
       }

       buf->retrieve(sizeof(MSG_Production));

       LOG_INFO << "Speed: " << Tranverse16(stuBody->Speed);
       LOG_INFO << "Production: " << Tranverse32(stuBody->Production);
       LOG_INFO << "AckCode: " << tmpAckCode;

       if (tmpAckCode != ACK_OK)
       {
         LOG_INFO << "exception: "<< tmpAckCode;
       }

       // sendReplyAck(get_pointer(conn),&stuBody->header,tmpAckCode);
    }
    else if (tHeader->MsgType == (MSG_REPLY|MSG_GETTASKINFO))
    {
      if(buf->readableBytes() < sizeof(MSG_TaskInfo))
      {
        buf->retrieve(buf->readableBytes());
        LOG_INFO << "-------- ACK_DATALOSS ";
        return;
      }

// }MSG_TaskInfo,*pMSG_TaskInfo;
       MSG_TaskInfo* stuBody = (MSG_TaskInfo*)const_cast<char*>(buf->peek());


       LOG_INFO << "cmd type: " << stuBody->header.MsgType;

        //报文合法性校验
       if(Tranverse16(stuBody->header.length) != sizeof(MSG_TaskInfo))
       {
          tmpAckCode = ACK_OUTOFMEM;
       }

       if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
       {
         tmpAckCode = ACK_DATALOSS;
       }

       if (stuBody->header.destAddr != m_localAddr)
       {
         tmpAckCode = ACK_MSG_ERROR;
       }

       buf->retrieve(sizeof(MSG_TaskInfo));

       LOG_INFO << "WorkNum: " << Tranverse16(stuBody->WorkNum);
       LOG_INFO << "ProductTask: " << Tranverse32(stuBody->ProductTask);
       LOG_INFO << "Class: " << (stuBody->Class);

       if (tmpAckCode != ACK_OK)
       {
        LOG_INFO << "exception: "<< tmpAckCode;
       }
    }
    else if (tHeader->MsgType == (MSG_REPLY|MSG_GETMACSTATE))
    {
      if(buf->readableBytes() < sizeof(MSG_MacState))
      {
        buf->retrieve(buf->readableBytes());
        LOG_INFO << "-------- ACK_DATALOSS ";
        return;
      }

// }MSG_MacState,*pMSG_MacState;

       MSG_MacState* stuBody = (MSG_MacState*)const_cast<char*>(buf->peek());


       LOG_INFO << "cmd type: " << stuBody->header.MsgType;

        //报文合法性校验
       if(Tranverse16(stuBody->header.length) != sizeof(MSG_MacState))
       {
          tmpAckCode = ACK_OUTOFMEM;
       }

       if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
       {
         tmpAckCode = ACK_DATALOSS;
       }

       if (stuBody->header.destAddr != m_localAddr)
       {
         tmpAckCode = ACK_MSG_ERROR;
       }

       buf->retrieve(sizeof(MSG_MacState));

       LOG_INFO << "MacState: " << (stuBody->MacState);
       LOG_INFO << "MacErr: " << (stuBody->MacErr);
       LOG_INFO << "StopTmLen: " << Tranverse32(stuBody->StopTmLen);

       if (tmpAckCode != ACK_OK)
       {
         LOG_INFO << "exception: "<< tmpAckCode;
       }
    }
    else if (tHeader->MsgType == (MSG_REPLY|MSG_GETPATPARA))
    {
      if(buf->readableBytes() < sizeof(MSG_PatPara))
      {
        buf->retrieve(buf->readableBytes());
        LOG_INFO << "-------- ACK_DATALOSS ";
        return;
      }

      // MSG_PatPara,*pMSG_PatPara;
      MSG_PatPara* stuBody = (MSG_PatPara*)const_cast<char*>(buf->peek());


      LOG_INFO << "cmd type: " << stuBody->header.MsgType;

      //报文合法性校验
      if(Tranverse16(stuBody->header.length) != sizeof(MSG_PatPara))
      {
         tmpAckCode = ACK_OUTOFMEM;
      }

      if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
      {
        tmpAckCode = ACK_DATALOSS;
      }

      if (stuBody->header.destAddr != m_localAddr)
      {
        tmpAckCode = ACK_MSG_ERROR;
      }

      buf->retrieve(sizeof(MSG_PatPara));

      LOG_INFO << "TotalWeft: " << Tranverse16(stuBody->TotalWeft);
      LOG_INFO << "Warp: " << Tranverse16(stuBody->Warp);
      LOG_INFO << "FileSize: " << Tranverse16(stuBody->FileSize);
      LOG_INFO << "StringLen: " << Tranverse16(stuBody->StringLen);
      LOG_INFO << "FileName: " << stuBody->FileName;

      if (tmpAckCode != ACK_OK)
      {
        LOG_INFO << "exception: "<< tmpAckCode;
      }
    }
    else if (tHeader->MsgType == (MSG_REPLY|MSG_GETMACINFO))
    {
      if(buf->readableBytes() < sizeof(MSG_MacInfo))
      {
        buf->retrieve(buf->readableBytes());
        LOG_INFO << "-------- ACK_DATALOSS ";
        return;
      }
      //MSG_MacInfo,*pMSG_MacInfo;

      MSG_MacInfo* stuBody = (MSG_MacInfo*)const_cast<char*>(buf->peek());


      LOG_INFO << "cmd type: " << stuBody->header.MsgType;

      //报文合法性校验
      if(Tranverse16(stuBody->header.length) != sizeof(MSG_MacInfo))
      {
         tmpAckCode = ACK_OUTOFMEM;
      }

      if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
      {
        tmpAckCode = ACK_DATALOSS;
      }

      if (stuBody->header.destAddr != m_localAddr)
      {
        tmpAckCode = ACK_MSG_ERROR;
      }

      buf->retrieve(sizeof(MSG_MacInfo));

      LOG_INFO << "Row: " << (stuBody->Row);
      LOG_INFO << "Col: " << (stuBody->Col);
      LOG_INFO << "WeftDensity: " << Tranverse16(stuBody->WeftDensity);
      LOG_INFO << "OpeningDegree: " << Tranverse16(stuBody->OpeningDegree);
      LOG_INFO << "OutNum: " << (stuBody->OutNum);
      LOG_INFO << "Installation: " << (stuBody->Installation);
      LOG_INFO << "CardSlot: " << (stuBody->CardSlot);

      if (tmpAckCode != ACK_OK)
      {
         // sendReplyAck(get_pointer(conn),&stuBody->header,tmpAckCode);
          LOG_INFO << "exception: "<< tmpAckCode;
      }
      
    }
    else
    {
      LOG_INFO << "###############---------unknown cmd----################";
      // disconnect 

      // char* pBuf = (char*)const_cast<char*>(buf->peek());
      // int data_len = buf->readableBytes();
      // char buffer[data_len+1];
      // buffer[data_len+1] = 0;

      // for(int j = 0; j < data_len; j++)
      //     sprintf(buffer[2*j], "%02X", pBuf[j]);

      int iLen = buf->readableBytes();
      LOG_INFO << "data_len: " << iLen;
      for (int i = 0; i < iLen; ++i)
      {
         printf("%02X ", buf->peek()[i]);
      }
      printf("\n");


    // LOG_INFO <<"MSG_Header: "<< ss.str();

      buf->retrieve(buf->readableBytes());

    }

    return;
   } 
  
}

UINT16 JacServer::getMsgSerialNo()
{
  if (m_curMsgSerialNo == MAX_SERIAL_NO)
  {

    m_curMsgSerialNo=0;
  }
  else
  {
    m_curMsgSerialNo++;
    LOG_INFO << "m_curMsgSerialNo++===========: " << m_curMsgSerialNo;
  }

  return m_curMsgSerialNo;

}

int kRollSize = 500*1000*1000;

boost::scoped_ptr<muduo::AsyncLogging> g_asyncLog;

void asyncOutput(const char* msg, int len)
{
  g_asyncLog->append(msg, len);
}

void setLogging(const char* argv0)
{
  muduo::Logger::setOutput(asyncOutput);
  char name[256];
  strncpy(name, argv0, 256);
  g_asyncLog.reset(new muduo::AsyncLogging(::basename(name), kRollSize));
  g_asyncLog->start();
}

int main(int argc, char* argv[])
{
  //setLogging(argv[0]);

  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  EventLoop loop;
  InetAddress listenAddr(2007);
  LOG_INFO << "Listening at: " << listenAddr.toPort();
  JacServer server(&loop, listenAddr);

  server.start();

  loop.loop();
}

