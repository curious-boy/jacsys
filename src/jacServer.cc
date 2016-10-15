#include "jacServer.h"
#include "database_operator.h"

#include "muduo/base/TimeZone.h"
#include "muduo/base/LogFile.h"
#include "muduo/base/Logging.h"

DatabaseOperator g_DatabaseOperator;

void JacServer::processDB()
{
    LOG_INFO << "... processDB ...";

    while (true)
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
    if (g_DatabaseOperator.Init() < 0)
    {
        LOG_ERROR << "Database Init failed! ";
        exit(-1);
    }

    threadPool_.run(boost::bind(&processDB));
#endif

    server_.start();
}

void JacServer::onConnection(const TcpConnectionPtr &conn)
{
    LOG_DEBUG << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected())
    {
        if (connections_.size() >= 1)
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
            string strip = conn->peerAddress().toIp();
            vnodes = g_DatabaseOperator.GetNodesOfGateway(strip);

            if (vnodes.size() > 0)
            {
                //锟斤拷锟节碉拷锟斤拷息锟斤拷锟接碉拷锟斤拷锟斤拷锟斤拷息锟叫憋拷
                m_localAddr = g_DatabaseOperator.GetZigAddrOfGateway(strip);
                if (m_localAddr == 0)
                {
                    return;
                }

                for (int i = 0; i < vnodes.size(); i++)
                {
                    pINFO_Node tmpNode = new INFO_Node();
                    tmpNode->addr = vnodes[i];
                    tmpNode->unReplyNum = 0;
                    LOG_DEBUG << ">>>>>t node by db";
                    m_curGateway->insertNode(tmpNode);
                }
                m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
            }
            else
            {
                LOG_DEBUG << "There are no node of getway " << strip << " be registered!!!";
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

    //循锟斤拷锟斤拷锟斤拷锟窖撅拷注锟斤拷锟侥节碉拷
    if (m_curGateway->getNodeSize() == 0)
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
        //未锟斤拷应锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷8时锟斤拷锟叫讹拷锟节碉拷锟斤拷锟竭ｏ拷锟斤拷锟斤拷锟斤拷锟斤拷息锟斤拷删锟斤拷锟矫节碉拷
        LOG_INFO << "+++++ getUnReplyNum , " << m_curGateway->getUnReplyNum();

        if (m_curGateway->getUnReplyNum() > MAX_UNREPLY_NUM)
        {
            pINFO_Node tnode = m_curGateway->getCurNode();

            LOG_INFO << "----- node, addr= " << tnode->addr << " was removed!";
            LOG_INFO << ">>>>>>before deleteNodeByAddr, size = " << m_curGateway->getNodeSize();

#if USE_DATABASE
            //insert fault record here 
            std::ostringstream ostrsql;
            ostrsql << "insert into fault_record (fault_type,machine_id) VALUES('" << 03 << "','" << tnode->macId << "');";

            LOG_DEBUG << "fault_record insert sql: " << ostrsql.str().c_str();

            DatabaseOperatorTask insert_task_2;
            insert_task_2.operator_type = 0;
            insert_task_2.content = ostrsql.str().c_str();
            g_DatabaseOperator.AddTask(insert_task_2);

            g_DatabaseOperator.DeleteNodeOfGateway(m_curGateway->getIp(), tnode->addr);
#endif
            m_curGateway->deleteNodeByAddr(tnode->addr);

            LOG_INFO << ">>>>>>after deleteNodeByAddr, size = " << m_curGateway->getNodeSize();

            if (m_curGateway->getNodeSize() == 0)
            {
                return;
            }

            //锟斤拷询锟斤拷一锟斤拷锟节碉拷锟侥碉拷址
            m_destAddr = m_curGateway->getNextNode()->addr;
            modifyDestAddr(m_destAddr);

            m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
            return;
        }

        UINT16 tmpCrc = 0;
        UINT16 tmpNo = getMsgSerialNo();
        LOG_INFO << "onTimer: tmpNo=" << tmpNo;

        if (times_get_mac_state_ < 10)
        {
            // #define MSG_GETMACSTATE       0X49  //锟斤拷取锟斤拷锟斤拷状态锟斤拷息
            msgLen = sizeof(MSG_GetMacState);
            MSG_GetMacState *stuGetMacState = (MSG_GetMacState *)new char[sizeof(MSG_GetMacState)];

            stuGetMacState->header.Sof = COM_FRM_HEAD;
            stuGetMacState->header.MsgType = MSG_GETMACSTATE;
            stuGetMacState->header.srcAddr = (m_localAddr);
            stuGetMacState->header.destAddr = Tranverse16(m_destAddr);
            stuGetMacState->header.length = Tranverse16(msgLen);
            stuGetMacState->header.serialNo = Tranverse16(tmpNo);
            stuGetMacState->header.replyNo = 0;
            stuGetMacState->header.crc16 = 0;
            stuGetMacState->Eof = COM_FRM_END;

            tmpCrc = CalcCRC16(0, &stuGetMacState->header.Sof, msgLen);
            stuGetMacState->header.crc16 = Tranverse16(tmpCrc);

            m_sendBuf.append(stuGetMacState, msgLen);
            times_get_mac_state_++;
        }

        else
        {
            msgLen = sizeof(MSG_GetProduction);
            MSG_GetProduction *stuGetProduction = (MSG_GetProduction *)new char[msgLen];

            stuGetProduction->header.Sof = COM_FRM_HEAD;
            stuGetProduction->header.MsgType = MSG_GETPRODUCTION;
            stuGetProduction->header.srcAddr = (m_localAddr);
            stuGetProduction->header.destAddr = Tranverse16(m_destAddr);
            stuGetProduction->header.length = Tranverse16(msgLen);
            stuGetProduction->header.serialNo = Tranverse16(tmpNo);
            stuGetProduction->header.replyNo = 0;
            stuGetProduction->header.crc16 = 0;
            stuGetProduction->Eof = COM_FRM_END;

            tmpCrc = CalcCRC16(0, &stuGetProduction->header.Sof, msgLen);
            stuGetProduction->header.crc16 = Tranverse16(tmpCrc);

            m_sendBuf.append(stuGetProduction, msgLen);
            times_get_mac_state_ = 0;
        }

        m_curGateway->setCurOperatorType(MODIFY_DEST_NODE); // for change operation type

        m_curGateway->increaseUnReplyNum();
        sendAll(&m_sendBuf);

        m_loop->cancel(m_roundTimer);
        m_roundTimer = m_loop->runAfter(ROUND_INTERVAL_SECONDS, boost::bind(&JacServer::onTimer, this));
    }
}

void JacServer::sendAll(Buffer *buf)
{
    if (connections_.size() != 1)
    {
        LOG_WARN << "!!!!!!!!!!!showld be only one connection !!!!,please check!!!";
    }

    int i = 0;
    for (ConnectionList::iterator it = connections_.begin();
         it != connections_.end();
         ++it)
    {
        get_pointer(*it)->setTcpNoDelay(true);
        get_pointer(*it)->send(buf);
    }

    m_iSendNo++;
}

void JacServer::sendReplyAck(TcpConnection *conn, pMSG_Header srcheader, UINT8 ACK_code)
{
    MSG_ACK stuAck;
    pMSG_Header pheader = &stuAck.header;
    UINT16 msgLen = sizeof(MSG_ACK);
    UINT16 tmpCrc = 0;

    // ack
    Buffer ackBuf;
    UINT16 tmpNo = getMsgSerialNo();

    pheader->Sof = COM_FRM_HEAD;
    pheader->MsgType = MSG_COMACK;
    pheader->srcAddr = srcheader->destAddr;
    pheader->destAddr = srcheader->srcAddr;
    pheader->length = Tranverse16(msgLen);
    LOG_DEBUG << "-----))))))))))))))))))))pheader->length: " << pheader->length;
    LOG_DEBUG << "-----))))))))))))))))))))msgLen: " << msgLen;

    pheader->serialNo = Tranverse16(tmpNo);
    pheader->replyNo = srcheader->serialNo;
    LOG_INFO << "----------serialNo: " << Tranverse16(pheader->serialNo);
    LOG_INFO << "---------replyNo: " << Tranverse16(pheader->replyNo);

    pheader->crc16 = 0;

    stuAck.AckCode = Tranverse16(ACK_code);
    stuAck.Eof = COM_FRM_END;

    tmpCrc = CalcCRC16(0, &stuAck.header.Sof, msgLen);
    pheader->crc16 = Tranverse16(tmpCrc);

    ackBuf.append(&stuAck, sizeof(MSG_ACK));

    conn->send(&ackBuf);
    LOG_DEBUG << "ack was sended!";
}

// process data/protocol
void JacServer::onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time)
{

    LOG_INFO << "readableBytes length = " << buf->readableBytes();

    MSG_Header *tHeader = (MSG_Header *)new char[sizeof(MSG_Header)];

    if (buf->readableBytes() >= sizeof(RspAck))
    {
        pRspAck tmpAck = (pRspAck) new char[sizeof(RspAck)];
        tmpAck = (pRspAck) const_cast<char *>(buf->peek());

        if ((tmpAck->protocolTag1 == 0xDE) && (tmpAck->protocolTag2 == 0xDF) && (tmpAck->protocolTag3 == 0xEF) && (tmpAck->funcCode == 0xD2))
        {
            //锟斤拷锟斤拷锟睫革拷目锟斤拷锟节碉拷锟斤拷锟筋反锟斤拷
            if (tmpAck->ackCode == 0x00)
            {
                m_loop->cancel(m_resendTimer);
                if (m_curGateway->getCurOperatorType() == REGISTER_NODE)
                {
                    sendReplyAck(get_pointer(conn), m_pTmpHeader, tmpAckCode_);

                    pINFO_Node tmpNode = new INFO_Node();
                    tmpNode->addr = m_pTmpHeader->srcAddr;
                    tmpNode->unReplyNum = 0;
                    m_curGateway->insertNode(tmpNode);

//锟斤拷锟节碉拷锟斤拷息锟斤拷锟诫到锟斤拷锟捷匡拷锟斤拷
#if USE_DATABASE
                    //machine_management
                    std::ostringstream ostrsql;
                    ostrsql << "insert into node_register_info (gateway_name, gateway_ip,gateway_zig_addr,node_zig_addr) VALUES ('','"
                            << m_curGateway->getIp() << "' ," << m_pTmpHeader->destAddr << "," << tmpNode->addr << ")";

                    LOG_DEBUG << "node_register_info insert sql: " << ostrsql.str().c_str();

                    DatabaseOperatorTask insert_task_1;
                    insert_task_1.operator_type = 0;
                    insert_task_1.content = ostrsql.str().c_str();
                    g_DatabaseOperator.AddTask(insert_task_1);

                    ostrsql.clear();
                    ostrsql << "INSERT INTO `jacdb`.`machine_management`(`machine_id`,`addr`,`gateway`,`machine_type`,`row`,`col`,`thread_number`,`Mcuversion`,`Universion`,`Hw1version`,`Hw2version`) VALUES('"
                            << m_pTmpMsgLogin->macID << "'," << m_pTmpHeader->destAddr << ",'" << m_pTmpMsgLogin->gatewayId << "','"
                            << m_pTmpMsgLogin->macType << "'," << m_pTmpMsgLogin->Row << "," << m_pTmpMsgLogin->Col << "," << m_pTmpMsgLogin->Warp << ","
                            << m_pTmpMsgLogin->McuVer << "," << m_pTmpMsgLogin->UiVer << "," << m_pTmpMsgLogin->Hw1Ver << "," << m_pTmpMsgLogin->Hw2Ver << ")";
                    LOG_DEBUG << "machine_management insert sql: " << ostrsql.str().c_str();

                    DatabaseOperatorTask insert_task_2;
                    insert_task_2.operator_type = 0;
                    insert_task_2.content = ostrsql.str().c_str();
                    g_DatabaseOperator.AddTask(insert_task_2);

//g_DatabaseOperator.InsertNodeOfGateway( m_curGateway->getIp(),m_pTmpHeader->destAddr,tmpNode->addr);
#endif

                    // m_curGateway->setCurOperatorType(REGISTER_FINISH);

                    if (m_destAddr == 0)
                    {
                        m_destAddr = Tranverse16(m_pTmpHeader->srcAddr);
                    }
                    LOG_DEBUG << "------REGISTER_NODE";

                    setNodeTime(m_destAddr); //锟节碉拷注锟斤拷锟缴癸拷锟斤拷锟斤拷同锟斤拷锟节碉拷时锟斤拷

                    if (m_curGateway->getNodeSize() > 0)
                    {
                        sleep(1);
                        modifyDestAddr(m_curGateway->getCurNode()->addr);
                    }
                    LOG_DEBUG << "---register final success----";
                    m_curGateway->setCurOperatorType(SEND_MESSAGE);

                    m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
                }
                else if (m_curGateway->getCurOperatorType() == MODIFY_DEST_NODE)
                {
                    LOG_DEBUG << "---------modify dest node success!-------";

                    m_curGateway->setCurOperatorType(SEND_MESSAGE);
                    m_loop->cancel(m_roundTimer);
                    m_roundTimer = m_loop->runAfter(1, boost::bind(&JacServer::onTimer, this));
                }

                else if (m_curGateway->getCurOperatorType() == LOGOUT_NODE)
                {
                    LOG_DEBUG << "---------LOGOUT_NODE ACK!-------";
                    sendReplyAck(get_pointer(conn), m_pTmpHeader, tmpAckCode_);
                    LOG_DEBUG << "---------LOGOUT_NODE ACK,sendReplyAck finished!-------";

                    // 锟叫讹拷锟角凤拷锟斤拷锟斤拷锟斤拷锟接的节碉拷
                    if (m_curGateway->getNodeSize() > 0)
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
                else if (m_curGateway->getCurOperatorType() == LOGOUT_FINISH)
                {
                    LOG_DEBUG << "---------LOGOUT_FINISH ACK!-------";
                    // 锟叫讹拷锟角凤拷锟斤拷锟斤拷锟斤拷锟接的节碉拷
                    if (m_curGateway->getNodeSize() > 0)
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

                if (m_curGateway->getCurOperatorType() == MODIFY_DEST_NODE)
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

    if (m_curGateway != NULL)
    {
        if ((m_curGateway->getCurOperatorType() == REGISTER_NODE) || (m_curGateway->getCurOperatorType() == REGISTER_NODE))
        {
            int iTempLen = buf->readableBytes();
            int iHeaderIndex = 0;
            int iEndIndex = 0;

            char *pTmpChar = const_cast<char *>(buf->peek());
            for (int i = 0; i < iTempLen; i++)
            {
                if ((UINT8)pTmpChar[i] == (UINT8)COM_FRM_HEAD)
                {
                    iHeaderIndex = i;
                }
            }
            for (int i = 0; i < iTempLen; i++)
            {
                if ((UINT8)pTmpChar[i] == (UINT8)COM_FRM_END)
                {
                    iEndIndex = i;
                }
            }
            if ((iHeaderIndex < iEndIndex) && (iEndIndex <= iTempLen))
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
        tHeader = (MSG_Header *)const_cast<char *>(buf->peek());

        if (tHeader->MsgType == MSG_LOGIN)
        {
            LOG_INFO << "===========MSG_LOGIN++++++++";
            LOG_DEBUG << "size of buf->readableBytes() :=" << buf->readableBytes();
            LOG_DEBUG << "sizeof(MSG_Login) := " << sizeof(MSG_Login);

            if (buf->readableBytes() < sizeof(MSG_Login))
            {
                sendReplyAck(get_pointer(conn), tHeader, ACK_DATALOSS);
                buf->retrieve(buf->readableBytes());
                return;
            }

            // MSG_Login
            tmpAckCode_ = ACK_OK;
            m_pTmpMsgLogin = (MSG_Login *)const_cast<char *>(buf->peek());

            if (m_localAddr == 0)
            {
                m_localAddr = (m_pTmpMsgLogin->header.destAddr);
                LOG_INFO << "---------destAddr: " << m_localAddr;
            }

            LOG_INFO << "-------------------srcAddr: " << (m_pTmpMsgLogin->header.srcAddr);
            LOG_INFO << "destAddr: " << (m_pTmpMsgLogin->header.destAddr);

            LOG_DEBUG << "length: " << Tranverse16(m_pTmpMsgLogin->header.length);
            LOG_DEBUG << "serialNo: " << Tranverse16(m_pTmpMsgLogin->header.serialNo);
            LOG_DEBUG << "replyNo: " << Tranverse16(m_pTmpMsgLogin->header.replyNo);
            LOG_DEBUG << "crc16: " << m_pTmpMsgLogin->header.crc16;

            LOG_DEBUG << "protocolVersion: " << Tranverse16(m_pTmpMsgLogin->protocolVersion);
            LOG_DEBUG << " StringLen: " << Tranverse16(m_pTmpMsgLogin->StringLen);

            LOG_INFO << "gatewayId: " << m_pTmpMsgLogin->gatewayId;
            LOG_DEBUG << "Row: " << (m_pTmpMsgLogin->Row);
            LOG_DEBUG << "Col: " << (m_pTmpMsgLogin->Col);
            LOG_DEBUG << "Warp: " << Tranverse16(m_pTmpMsgLogin->Warp);
            LOG_DEBUG << "Installation: " << (m_pTmpMsgLogin->Installation);
            LOG_DEBUG << "CardSlot: " << (m_pTmpMsgLogin->CardSlot);
            LOG_DEBUG << "macID: " << (m_pTmpMsgLogin->macID);
            LOG_DEBUG << "macType: " << (m_pTmpMsgLogin->macType);
            LOG_DEBUG << "McuVer: " << (m_pTmpMsgLogin->McuVer);
            LOG_DEBUG << "UiVer: " << (m_pTmpMsgLogin->UiVer);
            LOG_DEBUG << "Hw1Ver: " << (m_pTmpMsgLogin->Hw1Ver);
            LOG_DEBUG << "Hw2Ver: " << (m_pTmpMsgLogin->Hw2Ver);

            //锟斤拷锟侥合凤拷锟斤拷校锟斤拷
            if (Tranverse16(m_pTmpMsgLogin->header.length) != sizeof(MSG_Login))
            {
                tmpAckCode_ = ACK_OUTOFMEM;
                LOG_WARN << "ACK_OUTOFMEM";
            }

            if ((m_pTmpMsgLogin->header.Sof != COM_FRM_HEAD) || (m_pTmpMsgLogin->Eof != COM_FRM_END))
            {
                tmpAckCode_ = ACK_DATALOSS;
                LOG_WARN << "ACK_DATALOSS";
            }

            if (m_pTmpMsgLogin->header.destAddr != m_localAddr)
            {
                tmpAckCode_ = ACK_MSG_ERROR;
                LOG_WARN << "ACK_MSG_ERROR";
            }

            // 1 锟斤拷时取锟斤拷锟斤拷询锟斤拷时
            // 2 锟斤拷锟斤拷锟斤拷锟斤拷目锟斤拷锟节碉拷锟斤拷址为锟斤拷前注锟斤拷锟节碉拷锟斤拷址锟斤拷锟斤拷锟斤拷锟斤拷注锟斤拷锟缴癸拷应锟斤拷
            // 3 锟街革拷原锟斤拷询状态锟斤拷锟斤拷锟斤拷锟斤拷询锟斤拷
            if (m_curGateway == NULL)
            {
                m_curGateway = new Gateway();
                m_curGateway->setName(m_pTmpMsgLogin->gatewayId);
                LOG_INFO << "gatewayId: " << m_pTmpMsgLogin->gatewayId;
            }

            if (tmpAckCode_ == ACK_OK)
            {
                if (m_curGateway->isExistNode(m_pTmpMsgLogin->header.srcAddr))
                {
                    LOG_INFO << "-----------The node have been registed!!!!";
                    m_curGateway->deleteNodeByAddr(m_pTmpMsgLogin->header.srcAddr);
                    g_DatabaseOperator.DeleteNodeOfGateway(m_curGateway->getIp(), m_pTmpMsgLogin->header.srcAddr);
                }
            }
            else
            {
                LOG_INFO << "-----------The node registed failed!!!!";
                return;
            }

            if (m_pTmpHeader == NULL)
            {
                m_pTmpHeader = new MSG_Header();
            }

            m_pTmpHeader->destAddr = m_pTmpMsgLogin->header.destAddr;
            m_pTmpHeader->srcAddr = m_pTmpMsgLogin->header.srcAddr;
            m_pTmpHeader->serialNo = m_pTmpMsgLogin->header.serialNo;
            m_pTmpHeader->replyNo = m_pTmpMsgLogin->header.replyNo;

            m_curGateway->setCurOperatorType(REGISTER_NODE);
            m_loop->cancel(m_roundTimer);
            modifyDestAddr(m_pTmpMsgLogin->header.srcAddr);

            buf->retrieve(sizeof(MSG_Login));
        }
        else if (tHeader->MsgType == MSG_LOGOUT)
        {
            LOG_INFO << "===========ONMSG:   MSG_LOGOUT++++++++";
            //锟节碉拷注锟斤拷锟斤拷锟睫改节碉拷状态
            if (buf->readableBytes() < sizeof(MSG_Logout))
            {
                sendReplyAck(get_pointer(conn), tHeader, ACK_DATALOSS);
                buf->retrieve(buf->readableBytes());
                return;
            }
            //MSG_Logout* stuBody = (MSG_Logout*) new char[sizeof(MSG_Logout)];
            MSG_Logout *stuBody = (MSG_Logout *)const_cast<char *>(buf->peek());

            //锟斤拷锟侥合凤拷锟斤拷校锟斤拷
            if (Tranverse16(stuBody->header.length) != sizeof(MSG_Logout))
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
            // 锟节碉拷注锟斤拷
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

            if (m_pTmpHeader == NULL)
            {
                m_pTmpHeader = new MSG_Header();
            }
            m_pTmpHeader->destAddr = stuBody->header.destAddr;
            m_pTmpHeader->srcAddr = stuBody->header.srcAddr;
            m_pTmpHeader->serialNo = stuBody->header.serialNo;
            m_pTmpHeader->replyNo = stuBody->header.replyNo;
        }
        else if (tHeader->MsgType == MSG_COMACK)
        {
            //common ack
            // MSG_ACK
            LOG_INFO << "===========ONMSG:   MSG_COMACK++++++++";
            if (buf->readableBytes() < sizeof(MSG_ACK))
            {
                // sendReplyAck(get_pointer(conn),tHeader,ACK_DATALOSS);
                LOG_WARN << "data loss in common ack";
                buf->retrieve(buf->readableBytes());
                return;
            }
            //MSG_Logout* stuBody = (MSG_Logout*) new char[sizeof(MSG_Logout)];
            MSG_ACK *stuBody = (MSG_ACK *)const_cast<char *>(buf->peek());

            LOG_INFO << "锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷common ack,serialNo = " << stuBody->header.serialNo << " | "
                     << "replyNo = " << stuBody->header.replyNo;

            //锟斤拷锟侥合凤拷锟斤拷校锟斤拷
            if (Tranverse16(stuBody->header.length) != sizeof(MSG_ACK))
            {
                tmpAckCode_ = ACK_OUTOFMEM;
            }

            if ((stuBody->header.Sof != COM_FRM_HEAD) || (stuBody->Eof != COM_FRM_END))
            {
                tmpAckCode_ = ACK_DATALOSS;
            }

            if (stuBody->header.destAddr != m_localAddr)
            {
                LOG_WARN << "stuBody->header.destAddr != m_localAddr";
                LOG_WARN << "stuBody->header.destAddr=" << stuBody->header.destAddr << " | "
                         << "m_localAddr= " << m_localAddr;
                tmpAckCode_ = ACK_MSG_ERROR;
            }

            LOG_ERROR << "common ack, errCode = " << tmpAckCode_;
            m_curGateway->resetUnReplyNum(stuBody->header.srcAddr);

            buf->retrieve(sizeof(MSG_ACK));
        }
        else if (tHeader->MsgType == (MSG_REPLY | MSG_GETMACSTATE))
        {
            LOG_INFO << "===========ONMSG:   MSG_GETMACSTATE++++++++";
            if (buf->readableBytes() < sizeof(MSG_MacState))
            {
                buf->retrieve(buf->readableBytes());
                LOG_INFO << "-------- ACK_DATALOSS ";
                return;
            }

            MSG_MacState *stuBody = (MSG_MacState *)const_cast<char *>(buf->peek());

            //锟斤拷锟侥合凤拷锟斤拷校锟斤拷
            if (Tranverse16(stuBody->header.length) != sizeof(MSG_MacState))
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

            LOG_DEBUG << "Speed: " << Tranverse16(stuBody->Speed);
            LOG_DEBUG << "MacState: " << (stuBody->MacState);
            LOG_DEBUG << "MacErr: " << (stuBody->MacErr);
            LOG_DEBUG << "IdlTmLen: " << Tranverse32(stuBody->IdlTmLen);

            if (tmpAckCode_ != ACK_OK)
            {
                LOG_INFO << "exception: " << tmpAckCode_;
            }
            else
            {
                int timeInterval = 100;
                //机器故障0x01,说明机器断纱，向机器故障表中插入一条记录，故障类型为 01-机器断纱
                //机器状态为0,肯停机累计时长已经超过服务器设定的阈值，向机器故障表中插入一条故障数据，故障类型为01-机器断纱
                //服务端三次轮询节点无响应，则判断节点掉线，向机器故障表中插入一条故障数据，故障类型为 03-机器掉线
                //异常时同时向状态表和故障表中插入一条记录
                //machine_status 只插入
                //fault_record

                pINFO_Node pNode = m_curGateway->getNodeByAddr(stuBody->header.srcAddr);

                // process if state changed??
                pNode->machine_state = stuBody->MacState;
                pNode->broken_total_time = stuBody->IdlTmLen;
                pNode->halting_reason = stuBody->MacErr;

                m_curGateway->updateNodeByAddr(stuBody->header.srcAddr, pNode);

                // if( stuBody->MacErr == 0x01 )
                // {
                //    }

                if (stuBody->MacState == 0 && stuBody->IdlTmLen > timeInterval)
                {
                    pNode->halting_reason = 02;
                }

                // insert data to machine_status
                std::ostringstream ostrsql;
                ostrsql << "insert into machine_status (machine_id, machine_state,broken_total_time,halting_reason) VALUES ('" << pNode->macId << "'," << pNode->machine_state << "," << pNode->broken_total_time << "," << pNode->halting_reason << ");";

                LOG_DEBUG << "machine_status insert sql: " << ostrsql.str().c_str();

                DatabaseOperatorTask insert_task_1;
                insert_task_1.operator_type = 0;
                insert_task_1.content = ostrsql.str().c_str();
                g_DatabaseOperator.AddTask(insert_task_1);

                // insert data to fault record
                ostrsql.clear();
                ostrsql << "insert into fault_record (fault_type,machine_id) VALUES('" << pNode->halting_reason << "','" << pNode->macId << "');";

                LOG_DEBUG << "fault_record insert sql: " << ostrsql.str().c_str();

                DatabaseOperatorTask insert_task_2;
                insert_task_2.operator_type = 0;
                insert_task_2.content = ostrsql.str().c_str();
                g_DatabaseOperator.AddTask(insert_task_2);
            }
            m_curGateway->resetUnReplyNum(stuBody->header.srcAddr);
        }
        else if (tHeader->MsgType == (MSG_REPLY | MSG_GETPRODUCTION))
        {
            // get Production
            LOG_INFO << "===========ONMSG:   MSG_GETPRODUCTION++++++++";
            if (buf->readableBytes() < sizeof(MSG_Production))
            {
                buf->retrieve(buf->readableBytes());
                LOG_INFO << "-------- ACK_DATALOSS ";
                return;
            }

            MSG_Production *stuBody = (MSG_Production *)const_cast<char *>(buf->peek());

            //锟斤拷锟侥合凤拷锟斤拷校锟斤拷
            if (Tranverse16(stuBody->header.length) != sizeof(MSG_Production))
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

            LOG_DEBUG << "RunTmLen: " << Tranverse32(stuBody->RunTmLen);
            LOG_DEBUG << "Class: " << stuBody->Class;
            LOG_DEBUG << "WorkNum: " << stuBody->WorkNum;
            LOG_DEBUG << "ClassTmLen: " << Tranverse32(stuBody->ClassTmLen);
            LOG_DEBUG << "ClassOut: " << Tranverse32(stuBody->ClassOut);
            LOG_DEBUG << "TodayOut:" << Tranverse32(stuBody->TodayOut);
            LOG_DEBUG << "TodayTmLen:" << Tranverse32(stuBody->TodayTmLen);

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
                LOG_WARN << "exception: " << tmpAckCode_;
            }
            else
            {
                pINFO_Node pNode = m_curGateway->getNodeByAddr(stuBody->header.srcAddr);

                // UINT32 total_run_time;				//开机时长 单位：秒

                // std::string figure_name;			//花样名称
                // UINT32 latitude;
                // UINT32	opening;
                // UINT32 tasks_number;
                // UINT32 number_produced;
                // UINT32	how_long_to_finish;
                // UINT16	concurrent_produce_number;
                // std::string operator_num;
                // UINT32  product_total_time;
                // UINT32  product_total_output;

                // process if state changed??
                

                //节点产量信息协议@@@@ 后续需要关于时间判断的完善
                //machine_info 首次插入记录，后续只更新数据 如果注册时间的日期等于今天，则进行更新，否则进行插入@@
                std::ostringstream ostrsql;
                ostrsql << "insert into machine_info (machine_id, total_run_time,total_day_time,total_day_produced) VALUES ('" << pNode->macId << "'," << stuBody->RunTmLen << "," << stuBody->TodayTmLen << "," << stuBody->TodayOut << ");";

                LOG_DEBUG << "machine_info insert sql: " << ostrsql.str().c_str();

                DatabaseOperatorTask insert_task_1;
                insert_task_1.operator_type = 0;
                insert_task_1.content = ostrsql.str().c_str();
                g_DatabaseOperator.AddTask(insert_task_1);

                //figure_info 当节点机花样名称发生变化时，插入新记录，并记录花样更新时间到update_time字段，后续过程此字段保持不变
                //只更新其它字段 操作说明：当节点机的花样发生变化时，插入新记录，否则更新对应记录
                ostrsql.str("");
                if(pNode->figure_name != stuBody->FileName)
                {
                    ostrsql << "insert into figure_info (machine_id,register_time, figure_name,latitude,opening,tasks_number,number_produced,how_long_to_finish,concurrent_produce_number) VALUES ('" << pNode->macId << "'," << GetCurrentTime << ",'" << stuBody->FileName << "'," << stuBody->WeftDensity<<","<< stuBody->OpeningDegree<<","<<stuBody->PatTask<<","<<stuBody->TotalOut<<","<<stuBody->RemainTm<<","<<stuBody->OutNum << ");";

                    LOG_DEBUG << "figure_info insert sql: " << ostrsql.str().c_str();

                    DatabaseOperatorTask insert_task_1;
                    insert_task_1.operator_type = 0;
                    insert_task_1.content = ostrsql.str().c_str();
                    g_DatabaseOperator.AddTask(insert_task_1);
                }
                else
                {
                    //update record  @@@
                }

                //production_info 当值机工号发生变化时，插入新记录，并记录工号更新时间到update_time字段，后续过程该字段保持不变，
                //只更新其它字段； 操作类型：当节点机的值机工号发生变化时插入新记录，否则更新对应记录
                ostrsql.str("");
                if(pNode->operator_num != stuBody->WorkNum)
                {
                    //insert
                    ostrsql << "insert into production_info (machine_id,register_time, operator,product_total_time,product_total_output) VALUES ('" << pNode->macId << "'," << GetCurrentTime << ",'" << stuBody->WorkNum << "'," << stuBody->ClassTmLen<<","<< stuBody->ClassOut<<");";

                    LOG_DEBUG << "figure_info insert sql: " << ostrsql.str().c_str();

                    DatabaseOperatorTask insert_task_1;
                    insert_task_1.operator_type = 0;
                    insert_task_1.content = ostrsql.str().c_str();
                    g_DatabaseOperator.AddTask(insert_task_1);

                }
                else
                {
                    //update
                }
                
                //更新内存中的数据
                pNode->total_run_time = stuBody->RunTmLen;
                pNode->total_day_time = stuBody->TodayTmLen;
                pNode->total_day_produced = stuBody->TodayOut;

                pNode->figure_name = stuBody->FileName;
                pNode->latitude = stuBody->WeftDensity;
                pNode->opening = stuBody->OpeningDegree;
                pNode->tasks_number = stuBody->PatTask;
                pNode->number_produced = stuBody->TotalOut;
                pNode->how_long_to_finish = stuBody->RemainTm;
                pNode->concurrent_produce_number = stuBody->OutNum;
                pNode->operator_num = stuBody->WorkNum;
                pNode->product_total_time = stuBody->ClassTmLen;
                pNode->product_total_output = stuBody->ClassOut;

                m_curGateway->updateNodeByAddr(stuBody->header.srcAddr, pNode);
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

        m_curMsgSerialNo = 0;
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
    ModifyGateWayDestAddr *stuModifyGateWayDestAddr = (ModifyGateWayDestAddr *)new char(msgLen);
    stuModifyGateWayDestAddr->protocolTag1 = 0xDE;
    stuModifyGateWayDestAddr->protocolTag2 = 0xDF;
    stuModifyGateWayDestAddr->protocolTag3 = 0xEF;
    stuModifyGateWayDestAddr->funcCode = 0xD2;

    //LOG_INFO << "modifyDestAddr, test01";
    // UINT16 destAddr = m_curGateway->getNextNode()->addr;    // getNextNode锟斤拷要锟截革拷锟斤拷锟斤拷
    LOG_INFO << "^^^^^^^^^^^^^^^^^^ send modify dest node, addr = " << addr;
    stuModifyGateWayDestAddr->addr = addr;

    m_sendBuf.append(stuModifyGateWayDestAddr, msgLen);

    m_destAddr = Tranverse16(addr);
    sendAll(&m_sendBuf); // need modify

    m_loop->cancel(m_resendTimer);
    m_resendTimer = m_loop->runAfter(3, boost::bind(&JacServer::modifyDestAddr, this));
}

void JacServer::modifyDestAddr()
{
    LOG_INFO << "TIMER : modifyDestAddr...";
    UINT16 msgLen = 0;

    msgLen = sizeof(ModifyGateWayDestAddr);
    ModifyGateWayDestAddr *stuModifyGateWayDestAddr = (ModifyGateWayDestAddr *)new char(msgLen);
    stuModifyGateWayDestAddr->protocolTag1 = 0xDE;
    stuModifyGateWayDestAddr->protocolTag2 = 0xDF;
    stuModifyGateWayDestAddr->protocolTag3 = 0xEF;
    stuModifyGateWayDestAddr->funcCode = 0xD2;

    stuModifyGateWayDestAddr->addr = Tranverse16(m_destAddr);

    m_sendBuf.append(stuModifyGateWayDestAddr, msgLen);
    LOG_INFO << "^^^^^^^^^^^^^^^^^^ send modify dest node, addr = " << stuModifyGateWayDestAddr->addr;

    sendAll(&m_sendBuf); // need modify
    m_loop->cancel(m_resendTimer);
    m_resendTimer = m_loop->runAfter(3, boost::bind(&JacServer::modifyDestAddr, this));
}

void JacServer::setNodeTime(UINT16 addr)
{
    // #define MSG_SETTIME           0X44  //锟斤拷锟矫节碉拷时锟斤拷
    UINT16 tmpCrc = 0;

    UINT16 tmpNo = getMsgSerialNo();
    int msgLen = sizeof(MSG_SetTime);
    MSG_SetTime *stuSetTime = (MSG_SetTime *)new char[sizeof(MSG_SetTime)];

    stuSetTime->header.Sof = COM_FRM_HEAD;
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

    tmpCrc = CalcCRC16(0, &stuSetTime->header.Sof, msgLen);
    stuSetTime->header.crc16 = Tranverse16(tmpCrc);

    m_sendBuf.append(stuSetTime, msgLen);

    m_curGateway->increaseUnReplyNum();
    sendAll(&m_sendBuf);
}

void JacServer::updateTime()
{
    time_t now;
    time(&now);

    time_sync_ = localtime(&now);
}

int kRollSize = 500 * 1000 * 1000;

boost::scoped_ptr<muduo::AsyncLogging> g_asyncLog;

void asyncOutput(const char *msg, int len)
{
    g_asyncLog->append(msg, len);
}

void setLogging(const char *argv0)
{
    muduo::Logger::setOutput(asyncOutput);
    char name[256];
    strncpy(name, argv0, 256);
    g_asyncLog.reset(new muduo::AsyncLogging(::basename(name), kRollSize));
    g_asyncLog->start();
}


#define LISTEN_PORT 2007

boost::scoped_ptr<muduo::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
  g_logFile->append(msg, len);
}

void flushFunc()
{
  g_logFile->flush();
}

int main(int argc, char *argv[])
{
    int iport = LISTEN_PORT;

    char *p;
    long ltmp;
    if (argc > 1)
    {
        ltmp = strtol(argv[1], &p, 10);
        if (ltmp > 2000 && ltmp < 5000)
        {
            iport = ltmp;
        }
    }

    // write log to file
    std::ostringstream ostrlogfile;
    ostrlogfile << argv[0] << "." << iport;

    g_logFile.reset(new muduo::LogFile(::basename(ostrlogfile.str().c_str()), 200 * 1000));
    muduo::Logger::setOutput(outputFunc);
    muduo::Logger::setFlush(flushFunc);

    //set time config
    Logger::setLogLevel(Logger::DEBUG);
    muduo::TimeZone beijing(8 * 3600, "CST");
    Logger::setTimeZone(beijing);

    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    EventLoop loop;

    InetAddress listenAddr(iport);
    LOG_INFO << "Listening at: " << listenAddr.toPort();
    JacServer server(&loop, listenAddr, 2);

    server.start();

    loop.loop();
}
