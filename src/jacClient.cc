//#include "codec.h"

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/EventLoopThread.h>
#include <muduo/net/TcpClient.h>

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <iostream>
#include <stdio.h>

#include "MsgTypeDef.h"

using namespace muduo;
using namespace muduo::net;

class jacClient : boost::noncopyable
{
 public:
  jacClient(EventLoop* loop, const InetAddress& serverAddr)
    : client_(loop, serverAddr, "jacClient")
  {
    client_.setConnectionCallback(
        boost::bind(&jacClient::onConnection, this, _1));
    client_.setMessageCallback(
        boost::bind(&jacClient::onMessage, this, _1, _2, _3));
    client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  void disconnect()
  {
    client_.disconnect();
  }

struct teststu{
        int i;
        char name[2];
      }pteststu;


  void write(const StringPiece& message)
  {
    MutexLockGuard lock(mutex_);
    if (connection_)
    {
      // xuyao wanshan
      //codec_.send(get_pointer(connection_), message);

      Buffer buf;

      MSG_Login* stuLogin = (MSG_Login*)new char[sizeof(MSG_Login)];

      // MSG_Header* const tHeader = (MSG_Header*)(stuLogin->header);
      stuLogin->header.Sof=COM_FRM_HEAD;
      stuLogin->header.MsgType = MSG_LOGIN;
      stuLogin->header.srcAddr = 10;
      stuLogin->header.destAddr = 0;
      stuLogin->header.length = 100;
      stuLogin->header.serialNo = 1;
      stuLogin->header.replyNo = 2;
      stuLogin->header.crc16 = 165;

     // stuLogin->header = tHeader;
      // memcpy(stuLogin->header,tHeader,sizeof(MSG_Header));
      stuLogin->protocolVersion = 102;

      // char* strtmp = "fdasfda";
      // stuLogin.gatewayId = const_cast<char*>(strtmp);
      
      strcpy(stuLogin->gatewayId,"test10id");


      // char* tmpBuf = new char[sizeof(MSG_Login)];

      // memcpy(tmpBuf,stuLogin,sizeof(MSG_Login));

      
      LOG_INFO<< "stuLogin->gatewayId: "<< stuLogin->gatewayId;

    buf.append(stuLogin, sizeof(MSG_Login));


    // int32_t len = static_cast<int32_t>(message.size());
    // int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
    //buf.prepend(&be32, sizeof be32);

      LOG_INFO << "length of MSG_Login " << sizeof(MSG_Login);
      LOG_INFO << "send buf: "<< buf.readableBytes();

      connection_->send(&buf);

    }
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(mutex_);
    if (conn->connected())
    {
      connection_ = conn;
    }
    else
    {
      connection_.reset();
    }
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
  {
    string msg(buf->retrieveAllAsString());
  
  
  LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toString();
  conn->send(msg);

  }

  void onStringMessage(const TcpConnectionPtr&,
                       const string& message,
                       Timestamp)
  {
    printf("<<< %s\n", message.c_str());
  }

  TcpClient client_;
  //LengthHeaderCodec codec_;
  MutexLock mutex_;
  TcpConnectionPtr connection_;
};

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    port=2007;
    InetAddress serverAddr(argv[1], port);

    jacClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line))
    {
      
      client.write(line);
    }
    client.disconnect();
    CurrentThread::sleepUsec(1000*1000);  // wait for disconnect, see ace/logging/client.cc
  }
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}

