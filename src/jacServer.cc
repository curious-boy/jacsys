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

#include <sys/time.h>

#include "MsgTypeDef.h"
#include "tools.h"
#include "gateway.h"



using namespace muduo;
using namespace muduo::net;

class JacServer
{
public:
    JacServer(EventLoop* loop, const InetAddress& listenAddr)
        : m_loop(loop),
          server_(loop, listenAddr, "JacServer")
    {
        server_.setConnectionCallback(
            boost::bind(&JacServer::onConnection, this, _1));
        server_.setMessageCallback(
            boost::bind(&JacServer::onMessage, this, _1, _2, _3));

        m_loop->cancel(m_roundTimer);
        m_roundTimer = m_loop->runAfter(ROUND_INTERVAL_SECONDS,boost::bind(&JacServer::onTimer,this));

        m_curMsgSerialNo = 0;
        m_iSendNo = 0;
        m_curGateway = NULL;
        m_delayBuf = NULL;
        m_pTmpHeader = NULL;
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

    void modifyDestAddr(UINT16 addr);
    void modifyDestAddr();


    typedef std::set<TcpConnectionPtr> ConnectionList;
    EventLoop* m_loop;
    TcpServer server_;
    ConnectionList connections_;

    UINT16 m_curMsgSerialNo;
    UINT16 m_localAddr;

    //临时存放 目标地址
    UINT16 m_destAddr;
    UINT16 m_iSendNo;

    pMSG_Header m_pTmpHeader;       //临时存放消息头，用于新节点注册时的交互

    // tmp
    // std::vector<UINT16> m_vNodes;
    // std::map<std::string,Gateway> m_mGateways;
    Gateway*  m_curGateway;       //当前线程所处理的网关，目前只支持一个线程，一个网关

    TimerId   m_roundTimer;        //轮询定时器
    TimerId   m_resendTimer;        //指令重发定时器  修改目标节点地址

    Buffer*       m_delayBuf;         //缓存延迟处理的数据
    Buffer       m_sendBuf;

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
    LOG_DEBUG<< conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
        connections_.insert(conn);
    }
    else
    {
        connections_.erase(conn);
    }
}

void JacServer::onTimer()
{
    UINT16 msgLen = 0;
    LOG_INFO << "onTimer....";

    if (m_curGateway == NULL)
    {
        m_loop->cancel(m_roundTimer);
        m_roundTimer = m_loop->runAfter(ROUND_INTERVAL_SECONDS, boost::bind(&JacServer::onTimer, this));
        return;
    }

    //循环设置已经注册的节点
    if (m_curGateway->getNodeSize() == 0 )
    {
        LOG_INFO << "no node registed now!";
        m_loop->cancel(m_roundTimer);
        m_roundTimer = m_loop->runAfter(ROUND_INTERVAL_SECONDS, boost::bind(&JacServer::onTimer, this));
        return;
    }
    else if (m_curGateway->getCurOperatorType() == MODIFY_DEST_NODE)
    {

        UINT16 destAddr = m_curGateway->getNextNode()->addr;
        LOG_INFO << "%%%%%%%%%%%%%%%%%m_destAddr: " << destAddr;
        modifyDestAddr(destAddr);
        return;
    }
    else
    {

//未响应的命令数超过8时，判定节点掉线；从网关信息中删除该节点
        if(m_curGateway->getUnReplyNum() > MAX_UNREPLY_NUM)
        {
            LOG_INFO << "----- node, addr= "
                     << m_curGateway->getCurNode()->addr << " was removed!";
            m_curGateway->deleteNodeByAddr(m_curGateway->getCurNode()->addr);

            if(m_curGateway->getNodeSize() == 0)
            {
                return;
            }

//轮询下一个节点的地址
            m_destAddr= m_curGateway->getNextNode()->addr;
            modifyDestAddr(m_destAddr);

            m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
            return;
        }

        UINT16 tmpCrc=0;
        UINT8  tmpCmd = getSendCmd();
        UINT16 tmpNo = getMsgSerialNo();
        LOG_INFO << "onTimer: cmd="<< tmpCmd;
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

            m_sendBuf.append(stuGetProduction, msgLen);
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

            m_sendBuf.append(stuSetMacInfo, msgLen);
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

            m_sendBuf.append(stuGetMacInfo, msgLen);
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

            m_sendBuf.append(stuGetTaskInfo, msgLen);
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

            m_sendBuf.append(stuGetFirmWareInfo, msgLen);
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

            m_sendBuf.append(stuInterval, msgLen);
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

            m_sendBuf.append(stuSetTime, msgLen);
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

            m_sendBuf.append(stuGetMacState, msgLen);
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

            m_sendBuf.append(stuGetPatPara, msgLen);

            m_curGateway->setCurOperatorType(MODIFY_DEST_NODE);   // for change operation type
        }

        m_curGateway->increaseUnReplyNum();
        sendAll(&m_sendBuf);

        m_loop->cancel(m_roundTimer);
        m_roundTimer = m_loop->runAfter(ROUND_INTERVAL_SECONDS, boost::bind(&JacServer::onTimer, this));

    }

}

void JacServer::sendAll(Buffer* buf)
{
    LOG_DEBUG << "round buf size = " << buf->readableBytes();

    int i=0;
    for (ConnectionList::iterator it = connections_.begin();
         it != connections_.end();
         ++it)
    {
        get_pointer(*it)->setTcpNoDelay(true);
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
    UINT16 tmpNo = getMsgSerialNo();

    pheader->Sof=COM_FRM_HEAD;
    pheader->MsgType = MSG_COMACK;
    pheader->srcAddr = srcheader->destAddr;
    pheader->destAddr = srcheader->srcAddr;
    pheader->length = Tranverse16(msgLen);
    LOG_DEBUG << "-----))))))))))))))))))))pheader->length: " << pheader->length;
    LOG_DEBUG << "-----))))))))))))))))))))msgLen: " << msgLen;

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
    LOG_INFO <<"ack was sended!";
}

// process data/protocol
void JacServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{

    LOG_INFO << "readableBytes length = " << buf->readableBytes();

    MSG_Header* tHeader = (MSG_Header*) new char[sizeof(MSG_Header)];
    UINT8 tmpAckCode=ACK_OK;

    if (buf->readableBytes() >= sizeof(RspAck))
    {
        pRspAck tmpAck = (pRspAck) new char[sizeof(RspAck)];
        tmpAck = (pRspAck)const_cast<char*>(buf->peek());

        if ((tmpAck->protocolTag1 == 0xDE)
            && (tmpAck->protocolTag2 == 0xDF)
            && (tmpAck->protocolTag3 == 0xEF)
            && (tmpAck->funcCode == 0xD2 ) )
        {
            //处理修改目标节点命令反馈
            if (tmpAck->ackCode == 0x00)
            {
                m_loop->cancel( m_resendTimer );
                if(m_curGateway->getCurOperatorType() == REGISTER_NODE)
                {
                    sendReplyAck(get_pointer(conn),m_pTmpHeader,tmpAckCode);
                    m_curGateway->insertNodeFinished();
                    m_curGateway->setCurOperatorType(REGISTER_FINISH);

                    if (m_destAddr == 0)
                    {
                        m_destAddr = Tranverse16(m_pTmpHeader->srcAddr);
                    }
                }
                if(m_curGateway->getCurOperatorType() == REGISTER_FINISH)
                {

                    if (m_curGateway->getNodeSize() > 0 )
                    {
                        sleep(1);
                        modifyDestAddr(m_curGateway->getCurNode()->addr);
                    }
                    LOG_INFO << "RRRRRRRRRRRRRR---register final success----";
                    m_curGateway->setCurOperatorType(SEND_MESSAGE);

                    m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
                }
                if(m_curGateway->getCurOperatorType() == MODIFY_DEST_NODE)
                {
                    LOG_INFO << "---------modify dest node success!-------";

                    m_curGateway->setCurOperatorType(SEND_MESSAGE);
                    m_loop->cancel(m_roundTimer);
                    m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
                }
            }
            else
            {

                if(m_curGateway->getCurOperatorType() == MODIFY_DEST_NODE)
                {
                    LOG_INFO << "---------modify dest node failed!-------";
                    m_loop->cancel(m_roundTimer);

                    m_roundTimer = m_loop->runAfter(ROUND_INTERVAL_SECONDS, boost::bind(&JacServer::onTimer, this));

                }
                else
                {
                    LOG_INFO << "---------modifyDestAddr-------";
                    modifyDestAddr(m_pTmpHeader->srcAddr);
                }

            }

            buf->retrieve(sizeof(RspAck));
            return;
        }

    }

    if(m_curGateway != NULL)
    {
        if((m_curGateway->getCurOperatorType() == REGISTER_NODE)
           ||(m_curGateway->getCurOperatorType() == REGISTER_NODE))
        {
            int iTempLen = buf->readableBytes();
            int iHeaderIndex=0;
            int iEndIndex = 0;

            char * pTmpChar = const_cast<char*>(buf->peek());
            for(int i=0; i<iTempLen; i++)
            {
                if((UINT8)pTmpChar[i] == (UINT8)COM_FRM_HEAD)
                {
                    iHeaderIndex = i;
                }
            }
            for(int i=0; i<iTempLen; i++)
            {
                if((UINT8)pTmpChar[i] == (UINT8)COM_FRM_END)
                {
                    iEndIndex= i;
                }
            }
            if((iHeaderIndex< iEndIndex) && (iEndIndex <= iTempLen))
            {
                buf->retrieve(iEndIndex);
            }
            else
            {
                buf->retrieve(iTempLen);
            }

            LOG_INFO << "*******Registering..., throw bytes, length = " << iTempLen;
        }
    }

//如果当前为节点注册或注销，缓存数据
    //int iLen = buf->readableBytes();
    //LOG_INFO << "data_len: " << iLen;
    //for (int i = 0; i < iLen; ++i)
    //{
    //    printf("%02X ", buf->peek()[i]);
    //}
    //printf("\n");

    //if(m_curGateway != NULL )
    //{

    //    if(m_curGateway->getCurOperatorType() == REGISTER_NODE)
    //    {

    //        LOG_INFO << "@@@@@@@@@@@@@@@@@findCRLF: " << buf->findCRLF(0x7E);
    //        LOG_INFO << "findCRLF: " << buf->findCRLF();
    //        LOG_INFO << "findEOL: " << buf->findEOL();

    //        //包数据合法性检测
    //        // 1 找包头 2 找包尾 3 把中间数据缓存

    //        //     char RSP_HEADER[] = new char[4];
    //        //     RSP_HEADER[0] = 0xDE;
    //        //     RSP_HEADER[1] = 0xDF;
    //        //     RSP_HEADER[2] = 0xEF;
    //        //     RSP_HEADER[3] = 0xD2;

    //        // //  m_delayBuf->append(const char * data, size_t len);
    //        //    //buf->findCRLF( RSP_HEADER );
    //        //char* cacheData = const_cast<char*>(buf->peek());



    //    }
    //}

/////////////////////////////////////////////////////////////////////////
    if (buf->readableBytes() >= sizeof(MSG_Header))
    {
        /* code */
        tHeader = (MSG_Header*)const_cast<char*>(buf->peek());

        LOG_INFO << "onMessage, msgType: " << tHeader->MsgType;
        //cout << "msgtype:" <<hex<<tHeader<<endl;

        if (tHeader->MsgType == MSG_LOGIN)
        {
        	LOG_INFO << "===========MSG_LOGIN++++++++";
            if(buf->readableBytes() < sizeof(MSG_Login))
            {
                sendReplyAck(get_pointer(conn),tHeader,ACK_DATALOSS);
                buf->retrieve(buf->readableBytes());
                return;
            }

            // MSG_Login
            tmpAckCode=ACK_OK;
            MSG_Login* stuBody = (MSG_Login*)const_cast<char*>(buf->peek());

            if (m_localAddr == 0)
            {
                m_localAddr = (stuBody->header.destAddr);
            }
		
            LOG_INFO << "-------------------srcAddr: " << (stuBody->header.srcAddr);
            LOG_INFO << "destAddr: " << (stuBody->header.destAddr);

            LOG_DEBUG<< "length: " << Tranverse16(stuBody->header.length);
            LOG_DEBUG << "serialNo: " << Tranverse16(stuBody->header.serialNo);
            LOG_DEBUG << "replyNo: " << Tranverse16(stuBody->header.replyNo);
            LOG_DEBUG << "crc16: " << stuBody->header.crc16;

            LOG_DEBUG << "protocolVersion: " << Tranverse16(stuBody->protocolVersion);
            // LOG_INFO << " StringLen: " << Tranverse16(stuBody->StringLen);
            LOG_INFO << "gatewayId: " << stuBody->gatewayId;

            //报文合法性校验
            if(Tranverse16(stuBody->header.length) != sizeof(MSG_Login))
            {
                tmpAckCode = ACK_OUTOFMEM;
                LOG_WARN<< "ACK_OUTOFMEM";
            }

            if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
            {
                tmpAckCode = ACK_DATALOSS;
                LOG_WARN<< "ACK_DATALOSS";
            }

            if (stuBody->header.destAddr != m_localAddr)
            {
                tmpAckCode = ACK_MSG_ERROR;
                LOG_WARN<< "ACK_MSG_ERROR";
            }

            // 1 暂时取消轮询定时
            // 2 设置网关目标节点地址为当前注册节点地址，并发送注册成功应答
            // 3 恢复原轮询状态，继续轮询；
            if (m_curGateway == NULL)
            {
                m_curGateway = new Gateway();
                m_curGateway->setName(stuBody->gatewayId);
            }

            m_curGateway->setCurOperatorType( REGISTER_NODE);
            m_loop->cancel(m_roundTimer);
            modifyDestAddr(stuBody->header.srcAddr);

            if(m_pTmpHeader == NULL )
            {
                m_pTmpHeader = new MSG_Header();
            }
            m_pTmpHeader->destAddr= stuBody->header.destAddr;
            m_pTmpHeader->srcAddr= stuBody->header.srcAddr;
            m_pTmpHeader->serialNo = stuBody->header.serialNo;
            m_pTmpHeader->replyNo = stuBody->header.replyNo;

            //usleep(50000);
            //sendReplyAck(get_pointer(conn),&stuBody->header,tmpAckCode);
            //usleep(50000);

            // 节点注册成功
            if (tmpAckCode == ACK_OK)
            {
                if (!m_curGateway->isExistNode(stuBody->header.srcAddr))
                {
                    pINFO_Node tmpNode = new INFO_Node();      // when to free pointer?
                    tmpNode->addr = (stuBody->header.srcAddr);
                    tmpNode->unReplyNum = 0;

                    m_curGateway->insertNode(tmpNode);
                }
                else
                {
                    LOG_INFO << "-----------The node have been registed!!!!";
                }
            }
            else
            {
                LOG_INFO << "-----------The node registed failed!!!!";
            }

            buf->retrieve(sizeof(MSG_Login));

            //m_loop->cancel(m_roundTimer);
            //m_roundTimer = m_loop->runAfter(9, boost::bind(&JacServer::onTimer, this));

        }
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

            // add code to process logout   remove node info in Gateway
            // 节点注册成功
            if (tmpAckCode == ACK_OK)
            {

                if (m_curGateway->isExistNode(stuBody->header.srcAddr))
                {
                    m_curGateway->deleteNodeByAddr(stuBody->header.srcAddr);
                }
                else
                {
                    LOG_INFO << "-----------The node "
                             << stuBody->header.srcAddr
                             << " logout failed!";
                }
            }
            // ack
            // sendReplyAck(get_pointer(conn),&stuBody->header,tmpAckCode);

            if (m_curGateway == NULL)
            {
                m_curGateway = new Gateway();
                m_curGateway->setName(stuBody->gatewayId);
            }

            m_loop->cancel(m_roundTimer);
            modifyDestAddr(stuBody->header.srcAddr);
            usleep(30000);
            sendReplyAck(get_pointer(conn),&stuBody->header,tmpAckCode);
            // 需要修改
            modifyDestAddr(m_curGateway->getCurNode()->addr);
            m_loop->cancel(m_roundTimer);
            m_roundTimer = m_loop->runAfter(ROUND_INTERVAL_SECONDS, boost::bind(&JacServer::onTimer, this));
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

            LOG_INFO << "－－－－－－－common ack,serialNo = " << stuBody->header.serialNo << " | "
                     << "replyNo = " << stuBody->header.replyNo;

            //报文合法性校验
            if(Tranverse16(stuBody->header.length) != sizeof(MSG_ACK))
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


            LOG_DEBUG << "common ack, errCode = " << tmpAckCode;

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

            LOG_DEBUG<< "######### Speed: " << Tranverse16(stuBody->Speed);
            LOG_DEBUG << "Production: " << Tranverse32(stuBody->Production);
            LOG_DEBUG << "AckCode: " << tmpAckCode;

            if (tmpAckCode != ACK_OK)
            {
                LOG_WARN<< "exception: "<< tmpAckCode;
            }

            // sendReplyAck(get_pointer(conn),&stuBody->header,tmpAckCode);
        }
        else if (tHeader->MsgType == (MSG_REPLY|MSG_GETFIRMWAREINFO))
        {
            if(buf->readableBytes() < sizeof(MSG_FirmWareInfo))
            {
                buf->retrieve(buf->readableBytes());
                LOG_INFO << "-------- ACK_DATALOSS ";
                return;
            }

// }MSG_TaskInfo,*pMSG_TaskInfo;
            MSG_FirmWareInfo* stuBody = (MSG_FirmWareInfo*)const_cast<char*>(buf->peek());

            //报文合法性校验
            if(Tranverse16(stuBody->header.length) != sizeof(MSG_FirmWareInfo))
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

            buf->retrieve(sizeof(MSG_FirmWareInfo));

            LOG_DEBUG << "McuVer: " << (stuBody->McuVer);
            LOG_DEBUG << "UiVer: " << (stuBody->UiVer);
            LOG_DEBUG << "HwVer: " << (stuBody->HwVer);

            if (tmpAckCode != ACK_OK)
            {
                LOG_INFO << "exception: "<< tmpAckCode;
            }
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

            LOG_DEBUG << "WorkNum: " << Tranverse16(stuBody->WorkNum);
            LOG_DEBUG << "ProductTask: " << Tranverse32(stuBody->ProductTask);
            LOG_DEBUG << "Class: " << (stuBody->Class);

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

            LOG_DEBUG<< "MacState: " << (stuBody->MacState);
            LOG_DEBUG << "MacErr: " << (stuBody->MacErr);
            LOG_DEBUG << "StopTmLen: " << Tranverse32(stuBody->StopTmLen);

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

            LOG_DEBUG << "TotalWeft: " << Tranverse16(stuBody->TotalWeft);
            LOG_DEBUG << "Warp: " << Tranverse16(stuBody->Warp);
            LOG_DEBUG << "FileSize: " << Tranverse16(stuBody->FileSize);
            LOG_DEBUG << "StringLen: " << Tranverse16(stuBody->StringLen);
            LOG_DEBUG << "FileName: " << stuBody->FileName;

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


            LOG_INFO << "＃＃＃＃＃＃cmd type: " << stuBody->header.MsgType;

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

            LOG_DEBUG << "Row: " << (stuBody->Row);
            LOG_DEBUG << "Col: " << (stuBody->Col);
            LOG_DEBUG << "WeftDensity: " << Tranverse16(stuBody->WeftDensity);
            LOG_DEBUG << "OpeningDegree: " << Tranverse16(stuBody->OpeningDegree);
            LOG_DEBUG << "OutNum: " << (stuBody->OutNum);
            LOG_DEBUG << "Installation: " << (stuBody->Installation);
            LOG_DEBUG << "CardSlot: " << (stuBody->CardSlot);

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

//            int iLen = buf->readableBytes();
//            LOG_INFO << "data_len: " << iLen;
//            for (int i = 0; i < iLen; ++i)
//            {
//                printf("%02X ", buf->peek()[i]);
//            }
//            printf("\n");

            buf->retrieve(buf->readableBytes());
        }

        m_curGateway->resetUnReplyNum();
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

void JacServer::modifyDestAddr(UINT16 addr)
{
    UINT16 msgLen = 0;

    msgLen = sizeof(ModifyGateWayDestAddr);
    ModifyGateWayDestAddr* stuModifyGateWayDestAddr = (ModifyGateWayDestAddr*)new char(msgLen);
    stuModifyGateWayDestAddr->protocolTag1 = 0xDE;
    stuModifyGateWayDestAddr->protocolTag2 = 0xDF;
    stuModifyGateWayDestAddr->protocolTag3 = 0xEF;
    stuModifyGateWayDestAddr->funcCode = 0xD2;

    //LOG_INFO << "modifyDestAddr, test01";
    // UINT16 destAddr = m_curGateway->getNextNode()->addr;    // getNextNode不要重复调用
    LOG_INFO << "^^^^^^^^^^^^^^^^^^ send modify dest node, addr = " << addr;
    stuModifyGateWayDestAddr->addr = addr;

    m_sendBuf.append(stuModifyGateWayDestAddr,msgLen);

    m_destAddr = Tranverse16(addr);
    sendAll(&m_sendBuf);   // need modify

	m_loop->cancel(m_resendTimer);
    m_resendTimer = m_loop->runAfter(3,boost::bind(&JacServer::modifyDestAddr,this));
}

void JacServer::modifyDestAddr()
{
	LOG_INFO << "TIMER : modifyDestAddr...";
    UINT16 msgLen = 0;

    msgLen = sizeof(ModifyGateWayDestAddr);
    ModifyGateWayDestAddr* stuModifyGateWayDestAddr = (ModifyGateWayDestAddr*)new char(msgLen);
    stuModifyGateWayDestAddr->protocolTag1 = 0xDE;
    stuModifyGateWayDestAddr->protocolTag2 = 0xDF;
    stuModifyGateWayDestAddr->protocolTag3 = 0xEF;
    stuModifyGateWayDestAddr->funcCode = 0xD2;

    stuModifyGateWayDestAddr->addr = Tranverse16(m_destAddr);

    m_sendBuf.append(stuModifyGateWayDestAddr,msgLen);

    sendAll(&m_sendBuf);   // need modify
    m_loop->cancel(m_resendTimer);
    m_resendTimer = m_loop->runAfter(3,boost::bind(&JacServer::modifyDestAddr,this));
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
    Logger::setLogLevel(Logger::INFO);

    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    EventLoop loop;
    InetAddress listenAddr(2007);
    LOG_INFO << "Listening at: " << listenAddr.toPort();
    JacServer server(&loop, listenAddr);

    server.start();

    loop.loop();
}

