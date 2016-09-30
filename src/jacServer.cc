#include "jacServer.h"
#include "database_operator.h"


DatabaseOperator g_DatabaseOperator;

void JacServer::processDB()
{
    LOG_INFO << "... processDB ...";

    while(true)
    {
        g_DatabaseOperator.ExecTasks();

        usleep(50);
    }

}

void JacServer::start()
{
    LOG_INFO << "starting " << numThreads_ << " threads.";
    threadPool_.start(numThreads_);

#if USE_DATABASE
    // create dbthread here
    if(g_DatabaseOperator.Init() < 0)
    {
        LOG_ERROR << "Database Init failed! ";
        exit(-1);
    }

    threadPool_.run(boost::bind(&processDB));
#endif

    server_.start();
}

void JacServer::onConnection(const TcpConnectionPtr& conn)
{
    LOG_DEBUG<< conn->peerAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    if( conn->connected())
    {
        if(connections_.size() >= 1)
        {
            LOG_WARN << "!!!!!!!!!!!!!! only one gateway supperted. ";
            conn->shutdown();
        }
        else
        {
            connections_.insert(conn);

            if (m_curGateway == NULL)
            {
                m_curGateway = new Gateway();
                LOG_DEBUG << "onConnection,currentip :=" << conn->peerAddress().toIp();
                
                m_curGateway->setIp(conn->peerAddress().toIp());
            }

#if USE_DATABASE
            std::vector<UINT16> vnodes;
            string strip =conn->peerAddress().toIp();
            vnodes = g_DatabaseOperator.GetNodesOfGateway(strip );

            if(vnodes.size() > 0)
            {
                //���ڵ���Ϣ���ӵ�������Ϣ�б�
                m_localAddr=g_DatabaseOperator.GetZigAddrOfGateway(strip);
                if(m_localAddr == 0)
                {
                    return;
                }

                for(int i=0; i<vnodes.size(); i++)
                {
                    pINFO_Node tmpNode = new INFO_Node();
                    tmpNode->addr = vnodes[i];
                    tmpNode->unReplyNum = 0;
                    LOG_DEBUG<<">>>>>t node by db";
                    m_curGateway->insertNode(tmpNode);
                }
                m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
            }
            else
            {
                LOG_DEBUG<<"There are no node of getway "<<strip<<" be registered!!!";
            }
#endif
        }
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

    //ѭ�������Ѿ�ע���Ľڵ�
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
//δ��Ӧ������������8ʱ���ж��ڵ����ߣ���������Ϣ��ɾ���ýڵ�
        LOG_INFO << "+++++ getUnReplyNum , " << m_curGateway->getUnReplyNum();

        if(m_curGateway->getUnReplyNum() > MAX_UNREPLY_NUM)
        {
            pINFO_Node tnode = m_curGateway->getCurNode();

            LOG_INFO << "----- node, addr= "<< tnode->addr << " was removed!";
            LOG_INFO << ">>>>>>before deleteNodeByAddr, size = " << m_curGateway->getNodeSize();

#if USE_DATABASE
            g_DatabaseOperator.DeleteNodeOfGateway( m_curGateway->getIp(), tnode->addr);
#endif
            m_curGateway->deleteNodeByAddr(tnode->addr);

            LOG_INFO << ">>>>>>after deleteNodeByAddr, size = " << m_curGateway->getNodeSize();

            if(m_curGateway->getNodeSize() == 0)
            {
                return;
            }

//��ѯ��һ���ڵ��ĵ�ַ
            m_destAddr= m_curGateway->getNextNode()->addr;
            modifyDestAddr(m_destAddr);

            m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
            return;
        }

        UINT16 tmpCrc=0;
        UINT16 tmpNo = getMsgSerialNo();
        LOG_INFO << "onTimer: tmpNo=" <<tmpNo;


        if(times_get_mac_state_ < 10)
        {
            // #define MSG_GETMACSTATE       0X49  //��ȡ����״̬��Ϣ
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
            times_get_mac_state_++;
        }

        else
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
            times_get_mac_state_ =0;
        }


        m_curGateway->setCurOperatorType(MODIFY_DEST_NODE);   // for change operation type

        m_curGateway->increaseUnReplyNum();
        sendAll(&m_sendBuf);

        m_loop->cancel(m_roundTimer);
        m_roundTimer = m_loop->runAfter(ROUND_INTERVAL_SECONDS, boost::bind(&JacServer::onTimer, this));

    }

}

void JacServer::sendAll(Buffer* buf)
{
    if(connections_.size() != 1)
    {
        LOG_WARN << "!!!!!!!!!!!showld be only one connection !!!!,please check!!!";
    }

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
    
    conn->send(&ackBuf);
    LOG_DEBUG <<"ack was sended!";
}

// process data/protocol
void JacServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{

    LOG_INFO << "readableBytes length = " << buf->readableBytes();

    MSG_Header* tHeader = (MSG_Header*) new char[sizeof(MSG_Header)];

    if (buf->readableBytes() >= sizeof(RspAck))
    {
        pRspAck tmpAck = (pRspAck) new char[sizeof(RspAck)];
        tmpAck = (pRspAck)const_cast<char*>(buf->peek());

        if ((tmpAck->protocolTag1 == 0xDE)
            && (tmpAck->protocolTag2 == 0xDF)
            && (tmpAck->protocolTag3 == 0xEF)
            && (tmpAck->funcCode == 0xD2 ) )
        {
            //�����޸�Ŀ���ڵ������
            if (tmpAck->ackCode == 0x00)
            {
                m_loop->cancel( m_resendTimer );
                if(m_curGateway->getCurOperatorType() == REGISTER_NODE)
                {
                    sendReplyAck(get_pointer(conn),m_pTmpHeader,tmpAckCode_);

                    pINFO_Node tmpNode = new INFO_Node();
                    tmpNode->addr = m_pTmpHeader->srcAddr;
                    tmpNode->unReplyNum = 0;
                    m_curGateway->insertNode(tmpNode);

                    //���ڵ���Ϣ���뵽���ݿ���
#if USE_DATABASE
                    g_DatabaseOperator.InsertNodeOfGateway( m_curGateway->getIp(),m_pTmpHeader->destAddr,tmpNode->addr);
#endif

                    // m_curGateway->setCurOperatorType(REGISTER_FINISH);

                    if (m_destAddr == 0)
                    {
                        m_destAddr = Tranverse16(m_pTmpHeader->srcAddr);
                    }
                    LOG_DEBUG<<"------REGISTER_NODE";

                    setNodeTime(m_destAddr);    //�ڵ�ע���ɹ�����ͬ���ڵ�ʱ��

                    if (m_curGateway->getNodeSize() > 0 )
                    {
                        sleep(1);
                        modifyDestAddr(m_curGateway->getCurNode()->addr);
                    }
                    LOG_DEBUG << "---register final success----";
                    m_curGateway->setCurOperatorType(SEND_MESSAGE);

                    m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));

                }               
                else if(m_curGateway->getCurOperatorType() == MODIFY_DEST_NODE)
                {
                    LOG_DEBUG << "---------modify dest node success!-------";

                    m_curGateway->setCurOperatorType(SEND_MESSAGE);
                    m_loop->cancel(m_roundTimer);
                    m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
                }

                else if(m_curGateway->getCurOperatorType() == LOGOUT_NODE)
                {
                    LOG_DEBUG << "---------LOGOUT_NODE ACK!-------";
                    sendReplyAck(get_pointer(conn),m_pTmpHeader,tmpAckCode_);
                    LOG_DEBUG << "---------LOGOUT_NODE ACK,sendReplyAck finished!-------";

                    // �ж��Ƿ��������ӵĽڵ�
                    if (m_curGateway->getNodeSize() > 0 )
                    {
                        m_curGateway->resetCurNode();

                    }
                    else
                    {
                        return;
                    }

                    modifyDestAddr(m_curGateway->getCurNode()->addr);                
                    m_curGateway->setCurOperatorType(SEND_MESSAGE);
                    m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
                    LOG_DEBUG << "---logout finished----";
                }
                else if(m_curGateway->getCurOperatorType() == LOGOUT_FINISH)
                {
                    LOG_DEBUG << "---------LOGOUT_FINISH ACK!-------";
                    // �ж��Ƿ��������ӵĽڵ�
                    if (m_curGateway->getNodeSize() > 0 )
                    {
                        m_curGateway->resetCurNode();

                    }
                    else
                    {
                        return;
                    }

                    modifyDestAddr(m_curGateway->getCurNode()->addr);
                    m_loop->cancel(m_roundTimer);
                    LOG_DEBUG << "---logout finished----";
                    m_curGateway->setCurOperatorType(SEND_MESSAGE);
                    m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));

                }

            }
            else
            {
                LOG_DEBUG << "---------modify dest node failed!000-------";

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

/////////////////////////////////////////////////////////////////////////
    if (buf->readableBytes() >= sizeof(MSG_Header))
    {
        /* code */
        tHeader = (MSG_Header*)const_cast<char*>(buf->peek());

        if (tHeader->MsgType == MSG_LOGIN)
        {
            LOG_INFO << "===========MSG_LOGIN++++++++";
            LOG_DEBUG << "size of buf->readableBytes() :=" << buf->readableBytes();
            LOG_DEBUG << "sizeof(MSG_Login) := " << sizeof(MSG_Login);

            if(buf->readableBytes() < sizeof(MSG_Login))
            {
                sendReplyAck(get_pointer(conn),tHeader,ACK_DATALOSS);
                buf->retrieve(buf->readableBytes());
                return;
            }

            // MSG_Login
            tmpAckCode_=ACK_OK;
            MSG_Login* stuBody = (MSG_Login*)const_cast<char*>(buf->peek());

            if (m_localAddr == 0)
            {
                m_localAddr = (stuBody->header.destAddr);
                LOG_INFO << "---------destAddr: " << m_localAddr;
            }

            LOG_INFO << "-------------------srcAddr: " << (stuBody->header.srcAddr);
            LOG_INFO << "destAddr: " << (stuBody->header.destAddr);

            LOG_DEBUG<< "length: " << Tranverse16(stuBody->header.length);
            LOG_DEBUG << "serialNo: " << Tranverse16(stuBody->header.serialNo);
            LOG_DEBUG << "replyNo: " << Tranverse16(stuBody->header.replyNo);
            LOG_DEBUG << "crc16: " << stuBody->header.crc16;

            LOG_DEBUG << "protocolVersion: " << Tranverse16(stuBody->protocolVersion);
            LOG_DEBUG << " StringLen: " << Tranverse16(stuBody->StringLen);

            LOG_INFO << "gatewayId: " << stuBody->gatewayId;
            LOG_DEBUG << "Row: " << (stuBody->Row);
            LOG_DEBUG << "Col: " << (stuBody->Col);
            LOG_DEBUG << "Warp: " << Tranverse16(stuBody->Warp);
            LOG_DEBUG << "Installation: " << (stuBody->Installation);
            LOG_DEBUG << "CardSlot: " << (stuBody->CardSlot);
            LOG_DEBUG << "macID: " << (stuBody->macID);
            LOG_DEBUG << "macType: " << (stuBody->macType);
            LOG_DEBUG << "McuVer: " << (stuBody->McuVer);
            LOG_DEBUG << "UiVer: " << (stuBody->UiVer);
            LOG_DEBUG << "Hw1Ver: " << (stuBody->Hw1Ver);
            LOG_DEBUG << "Hw2Ver: " << (stuBody->Hw2Ver);


            //���ĺϷ���У��
            if(Tranverse16(stuBody->header.length) != sizeof(MSG_Login))
            {
                tmpAckCode_ = ACK_OUTOFMEM;
                LOG_WARN<< "ACK_OUTOFMEM";
            }

            if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
            {
                tmpAckCode_ = ACK_DATALOSS;
                LOG_WARN<< "ACK_DATALOSS";
            }

            if (stuBody->header.destAddr != m_localAddr)
            {
                tmpAckCode_ = ACK_MSG_ERROR;
                LOG_WARN<< "ACK_MSG_ERROR";
            }

            // 1 ��ʱȡ����ѯ��ʱ
            // 2 ��������Ŀ���ڵ���ַΪ��ǰע���ڵ���ַ��������ע���ɹ�Ӧ��
            // 3 �ָ�ԭ��ѯ״̬��������ѯ��
            if (m_curGateway == NULL)
            {
                m_curGateway = new Gateway();
                m_curGateway->setName(stuBody->gatewayId);
                LOG_INFO<<"gatewayId: "<<stuBody->gatewayId;

            }

            if (tmpAckCode_ == ACK_OK)
            {
                if (m_curGateway->isExistNode(stuBody->header.srcAddr))
                {
                    LOG_INFO << "-----------The node have been registed!!!!";
                    m_curGateway->deleteNodeByAddr(stuBody->header.srcAddr);
                    g_DatabaseOperator.DeleteNodeOfGateway(m_curGateway->getIp(), stuBody->header.srcAddr);
                }
            }
            else
            {
                LOG_INFO << "-----------The node registed failed!!!!";
                return;
            }


            if(m_pTmpHeader == NULL )
            {
                m_pTmpHeader = new MSG_Header();
            }

            m_pTmpHeader->destAddr= stuBody->header.destAddr;
            m_pTmpHeader->srcAddr= stuBody->header.srcAddr;
            m_pTmpHeader->serialNo = stuBody->header.serialNo;
            m_pTmpHeader->replyNo = stuBody->header.replyNo;

            m_curGateway->setCurOperatorType( REGISTER_NODE);
            m_loop->cancel(m_roundTimer);
            modifyDestAddr(stuBody->header.srcAddr);

            buf->retrieve(sizeof(MSG_Login));
        }
        else if (tHeader->MsgType == MSG_LOGOUT)
        {
            LOG_INFO << "===========ONMSG:   MSG_LOGOUT++++++++";
            //�ڵ�ע�����޸Ľڵ�״̬
            if(buf->readableBytes() < sizeof(MSG_Logout))
            {
                sendReplyAck(get_pointer(conn),tHeader,ACK_DATALOSS);
                buf->retrieve(buf->readableBytes());
                return;
            }
            //MSG_Logout* stuBody = (MSG_Logout*) new char[sizeof(MSG_Logout)];
            MSG_Logout* stuBody = (MSG_Logout*)const_cast<char*>(buf->peek());

            //���ĺϷ���У��
            if(Tranverse16(stuBody->header.length) != sizeof(MSG_Logout))
            {
                tmpAckCode_ = ACK_OUTOFMEM;
            }

            if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
            {
                tmpAckCode_ = ACK_DATALOSS;
            }

            if (stuBody->header.destAddr != m_localAddr)
            {
                tmpAckCode_ = ACK_MSG_ERROR;
            }

            buf->retrieve(sizeof(MSG_Logout));

            // add code to process logout   remove node info in Gateway
            // �ڵ�ע��
            if (tmpAckCode_ == ACK_OK)
            {
                if (m_curGateway->isExistNode(stuBody->header.srcAddr))
                {
                    m_curGateway->deleteNodeByAddr(stuBody->header.srcAddr);
                    g_DatabaseOperator.DeleteNodeOfGateway(m_curGateway->getIp(), stuBody->header.srcAddr);
                }
                else
                {
                    LOG_INFO << "-----------The node "
                             << stuBody->header.srcAddr
                             << " logout failed!";
                }
            }


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


        }
        else if (tHeader->MsgType == MSG_COMACK)
        {
            //common ack
            // MSG_ACK
            LOG_INFO << "===========ONMSG:   MSG_COMACK++++++++";
            if(buf->readableBytes() < sizeof(MSG_ACK))
            {
                // sendReplyAck(get_pointer(conn),tHeader,ACK_DATALOSS);
                LOG_WARN << "data loss in common ack" ;
                buf->retrieve(buf->readableBytes());
                return;
            }
            //MSG_Logout* stuBody = (MSG_Logout*) new char[sizeof(MSG_Logout)];
            MSG_ACK* stuBody = (MSG_ACK*)const_cast<char*>(buf->peek());

            LOG_INFO << "��������������common ack,serialNo = " << stuBody->header.serialNo << " | "
                     << "replyNo = " << stuBody->header.replyNo;

            //���ĺϷ���У��
            if(Tranverse16(stuBody->header.length) != sizeof(MSG_ACK))
            {
                tmpAckCode_ = ACK_OUTOFMEM;
            }

            if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
            {
                tmpAckCode_ = ACK_DATALOSS;
            }

            if (stuBody->header.destAddr != m_localAddr)
            {
                LOG_WARN<<"stuBody->header.destAddr != m_localAddr";
                LOG_WARN<<"stuBody->header.destAddr="<< stuBody->header.destAddr<<" | "
                        <<"m_localAddr= "<<m_localAddr;
                tmpAckCode_ = ACK_MSG_ERROR;
            }


            LOG_ERROR << "common ack, errCode = " << tmpAckCode_;
            m_curGateway->resetUnReplyNum(stuBody->header.srcAddr);

            buf->retrieve(sizeof(MSG_ACK));
        }
        else if (tHeader->MsgType == (MSG_REPLY|MSG_GETMACSTATE))
        {
            LOG_INFO << "===========ONMSG:   MSG_GETMACSTATE++++++++";
            if(buf->readableBytes() < sizeof(MSG_MacState))
            {
                buf->retrieve(buf->readableBytes());
                LOG_INFO << "-------- ACK_DATALOSS ";
                return;
            }


            MSG_MacState* stuBody = (MSG_MacState*)const_cast<char*>(buf->peek());

            //���ĺϷ���У��
            if(Tranverse16(stuBody->header.length) != sizeof(MSG_MacState))
            {
                tmpAckCode_ = ACK_OUTOFMEM;
            }

            if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
            {
                tmpAckCode_ = ACK_DATALOSS;
            }

            if (stuBody->header.destAddr != m_localAddr)
            {
                tmpAckCode_ = ACK_MSG_ERROR;
            }

            buf->retrieve(sizeof(MSG_MacState));

            LOG_DEBUG<<"Speed: " << Tranverse16(stuBody->Speed);
            LOG_DEBUG<< "MacState: " << (stuBody->MacState);
            LOG_DEBUG << "MacErr: " << (stuBody->MacErr);
            LOG_DEBUG << "IdlTmLen: " << Tranverse32(stuBody->IdlTmLen);

            if (tmpAckCode_ != ACK_OK)
            {
                LOG_INFO << "exception: "<< tmpAckCode_;
            }
            m_curGateway->resetUnReplyNum(stuBody->header.srcAddr);

        }
        else if (tHeader->MsgType == (MSG_REPLY|MSG_GETPRODUCTION))
        {
            // get Production
            LOG_INFO << "===========ONMSG:   MSG_GETPRODUCTION++++++++";
            if(buf->readableBytes() < sizeof(MSG_Production))
            {
                buf->retrieve(buf->readableBytes());
                LOG_INFO << "-------- ACK_DATALOSS ";
                return;
            }

            MSG_Production* stuBody = (MSG_Production*)const_cast<char*>(buf->peek());

            //���ĺϷ���У��
            if(Tranverse16(stuBody->header.length) != sizeof(MSG_Production))
            {
                tmpAckCode_ = ACK_OUTOFMEM;
            }
            else if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
            {
                tmpAckCode_ = ACK_DATALOSS;
            }
            else if (stuBody->header.destAddr != m_localAddr)
            {
                tmpAckCode_ = ACK_MSG_ERROR;
            }

            buf->retrieve(sizeof(MSG_Production));

            LOG_DEBUG<< "RunTmLen: " << Tranverse32(stuBody->RunTmLen);
            LOG_DEBUG << "Class: " << stuBody->Class;
            LOG_DEBUG << "WorkNum: " << stuBody->WorkNum;
            LOG_DEBUG << "ClassTmLen: " << Tranverse32(stuBody->ClassTmLen);
            LOG_DEBUG << "ClassOut: " << Tranverse32(stuBody->ClassOut);
            LOG_DEBUG << "PatTask: " << Tranverse32(stuBody->PatTask);
            LOG_DEBUG << "TotalOut: " << Tranverse32(stuBody->TotalOut);
            LOG_DEBUG << "RemainTm: " << Tranverse32(stuBody->RemainTm);

            LOG_DEBUG << "OutNum: " << stuBody->OutNum;
            LOG_DEBUG << "WeftDensity: " << Tranverse16(stuBody->WeftDensity);
            LOG_DEBUG << "OpeningDegree: " << Tranverse16(stuBody->OpeningDegree);
            LOG_DEBUG << "TotalWeft: " << Tranverse16(stuBody->TotalWeft);
            LOG_DEBUG << "StringLen: " << Tranverse16(stuBody->StringLen);
            LOG_DEBUG << "FileName: " << stuBody->FileName;

            LOG_DEBUG << "AckCode: " << tmpAckCode_;



            if (tmpAckCode_ != ACK_OK)
            {
                LOG_WARN<< "exception: "<< tmpAckCode_;
            }
            m_curGateway->resetUnReplyNum(stuBody->header.srcAddr);

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
    // UINT16 destAddr = m_curGateway->getNextNode()->addr;    // getNextNode��Ҫ�ظ�����
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
    LOG_INFO << "^^^^^^^^^^^^^^^^^^ send modify dest node, addr = " << stuModifyGateWayDestAddr->addr;

    sendAll(&m_sendBuf);   // need modify
    m_loop->cancel(m_resendTimer);
    m_resendTimer = m_loop->runAfter(3,boost::bind(&JacServer::modifyDestAddr,this));
}

void    JacServer::setNodeTime(UINT16 addr)
{
    // #define MSG_SETTIME           0X44  //���ýڵ�ʱ��
    UINT16 tmpCrc=0;

    UINT16 tmpNo = getMsgSerialNo();
    int  msgLen = sizeof(MSG_SetTime);
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

    updateTime();
    stuSetTime->hour = time_sync_->tm_hour;
    stuSetTime->minute = time_sync_->tm_min;
    stuSetTime->second = time_sync_->tm_sec;
    stuSetTime->date = time_sync_->tm_yday;
    stuSetTime->month = time_sync_->tm_mon;
    stuSetTime->year = Tranverse16(time_sync_->tm_year);

    stuSetTime->Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0,&stuSetTime->header.Sof,msgLen);
    stuSetTime->header.crc16 = Tranverse16(tmpCrc);

    m_sendBuf.append(stuSetTime, msgLen);

    m_curGateway->increaseUnReplyNum();
    sendAll(&m_sendBuf);
}

void   JacServer::updateTime()
{
    time_t now;
    time(&now);

    time_sync_= localtime(&now);
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

#define LISTEN_PORT 2007

int main(int argc, char* argv[])
{
    //setLogging(argv[0]);
    int iport=LISTEN_PORT;

    char *p;
    long ltmp;
    if(argc>1 )
    {
        ltmp = strtol(argv[1], &p, 10);
        if(ltmp>2000 && ltmp<5000)
        {
            iport=ltmp;
        }
    }

    Logger::setLogLevel(Logger::INFO);

    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    EventLoop loop;

    InetAddress listenAddr(iport);
    LOG_INFO << "Listening at: " << listenAddr.toPort();
    JacServer server(&loop, listenAddr,2);

    server.start();

    loop.loop();
}
