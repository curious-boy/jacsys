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
    ThreadPool threadPool_;             // �̳߳� ����Ϊ�����߳� ���̴߳�������io�����ݿ��̴߳�������
    int         numThreads_;                //�߳��� 2

    UINT16 m_curMsgSerialNo;            //��ǰ��Ϣ���
    UINT16 m_localAddr;                     //���صĵ�ַ

    //��ʱ��� Ŀ���ַ
    UINT16  m_destAddr;
    UINT16  m_iSendNo;
    UINT8 tmpAckCode_;

    pMSG_Header m_pTmpHeader;       //��ʱ�����Ϣͷ�������½ڵ�ע��ʱ�Ľ���
    MSG_Login*  m_pTmpMsgLogin;

    Gateway*  m_curGateway;       //��ǰ�߳�����������أ�Ŀǰֻ֧��һ���̣߳�һ������
   // INFO_Node node_;

    TimerId   m_roundTimer;        //��ѯ��ʱ��
    TimerId   m_resendTimer;        //ָ���ط���ʱ��  �޸�Ŀ��ڵ��ַ

    Buffer*       m_delayBuf;         //�����ӳٴ��������
    Buffer          m_sendBuf;

    tm*    time_sync_;         //��ǰʱ��

    int         times_get_mac_state_;       //��ȡ�ڵ�״̬���������ÿ��ѯʮ�Σ���ѯһ�νڵ������Ϣ
    

};

#endif
