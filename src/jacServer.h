#ifndef JAC_SERVER_H
#define JAC_SERVER_H

#include <muduo/net/TcpServer.h>

#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Mutex.h>
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

typedef struct 
{
    UINT16  task_type;                      //different task ,different select cause;
    UINT8    operator_type;             // insert ,select ,update ,and so on
    char[STRING_MAXLEN]     content;        // detailed content

}DatabaseOperatorTask, *pDatabaseOperatorTask; 

using namespace muduo;
using namespace muduo::net;

class JacServer
{
public:
    JacServer(EventLoop* loop, const InetAddress& listenAddr, int numThreads)
        : m_loop(loop),
          server_(loop, listenAddr, "JacServer"),
          numThreads_(numThreads),
          task_list_mutex_();

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
    }

    void start()
    {
        LOG_INFO << "starting " << numThreads_ << " threads.";
        threadPool_.start(numThreads_);

        // create dbthread here
        threadPool_.run(boost::bind(&processDB));
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

    void    modifyDestAddr(UINT16 addr);
    void    modifyDestAddr();

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

    pMSG_Header m_pTmpHeader;       //��ʱ�����Ϣͷ�������½ڵ�ע��ʱ�Ľ���

    Gateway*  m_curGateway;       //��ǰ�߳�����������أ�Ŀǰֻ֧��һ���̣߳�һ������

    TimerId   m_roundTimer;        //��ѯ��ʱ��
    TimerId   m_resendTimer;        //ָ���ط���ʱ��  �޸�Ŀ��ڵ��ַ

    Buffer*       m_delayBuf;         //�����ӳٴ��������
    Buffer          m_sendBuf;

    typedef std::vector<DatabaseOperatorTask> DatabaseOperatorTaskList;
    static DatabaseOperatorTaskList tasks_;                // database task list
    Mutex       task_list_mutex_;                               //

};

#endif