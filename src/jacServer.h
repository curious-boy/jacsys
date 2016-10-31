#ifndef JAC_SERVER_H
#define JAC_SERVER_H

#include <muduo/net/TcpServer.h>

#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/base/ThreadPool.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>

#include <boost/bind.hpp>

#include <utility>

#include <set>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <string>

#include <sys/time.h>

#include "MsgTypeDef.h"
#include "tools.h"
#include "gateway.h"

#include "XMLConfig.h"


#define USE_DATABASE 1

using namespace muduo;
using namespace muduo::net;


class JacServer
{
public:
    JacServer(EventLoop* loop, const InetAddress& listenAddr, int numThreads)
        : m_loop(loop),
          server_(loop, listenAddr, "JacServer"),
          numThreads_(numThreads)         
    {
        server_.setConnectionCallback(
                boost::bind(&JacServer::onConnection, this, _1));
        server_.setMessageCallback(
                boost::bind(&JacServer::onMessage, this, _1, _2, _3));

        m_curMsgSerialNo = 0;
        m_iSendNo = 0;
        m_curGateway = NULL;
        m_delayBuf = NULL;
        m_pTmpHeader = NULL;
        m_pTmpMsgLogin = NULL;
        time_sync_ = NULL;
        times_get_mac_state_=0;
        tmpAckCode_=ACK_OK;
        m_localAddr=0;
    }

    void start();
        
private:
    void onConnection(const TcpConnectionPtr& conn);

    void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time);

    void onTimer();

    void sendAll(Buffer* buf);

    void sendReplyAck(TcpConnection* conn, pMSG_Header srcheader,UINT8 ACK_code);

    UINT16 getMsgSerialNo();

    void    modifyDestAddr(UINT16 addr);
    void    modifyDestAddr();

    void    setNodeTime(UINT16 addr);
    void    updateTime();

    static void processDB();

    typedef std::set<TcpConnectionPtr> ConnectionList;
    EventLoop* m_loop;
    TcpServer server_;
    ConnectionList connections_;
    ThreadPool threadPool_;             // 线程池 设置为两个线程 主线程处理网络io，数据库线程处理数据
    int         numThreads_;                //线程数 2

    UINT16 m_curMsgSerialNo;            //当前消息序号
    UINT16 m_localAddr;                     //本地的地址

    //临时存放 目标地址
    UINT16  m_destAddr;
    UINT16  m_iSendNo;
    UINT8 tmpAckCode_;

    pMSG_Header m_pTmpHeader;       //临时存放消息头，用于新节点注册时的交互
    MSG_Login*  m_pTmpMsgLogin;

    Gateway*  m_curGateway;       //当前线程所处理的网关，目前只支持一个线程，一个网关
   // INFO_Node node_;

    TimerId   m_roundTimer;        //轮询定时器
    TimerId   m_resendTimer;        //指令重发定时器  修改目标节点地址

    Buffer*       m_delayBuf;         //缓存延迟处理的数据
    Buffer          m_sendBuf;

    tm*    time_sync_;         //当前时间

    int         times_get_mac_state_;       //获取节点状态命令次数，每轮询十次，轮询一次节点产量信息
    

};

#endif
