// the header of class Gateway
#ifndef GATEWAY_H
#define GATEWAY_H

#include "StuDef.h"
#include <string>
#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
// using namespace std;


typedef enum
{
    REGISTER_NODE,
    REGISTER_FINISH,
    MODIFY_DEST_NODE,
    SEND_MESSAGE

} OperatorType;

class Gateway
{
public:
    Gateway();
    ~Gateway();


public:
    pINFO_Node         getNodeByAddr(UINT16 addr);         // get nodeinfo by addr
    void                    insertNode(pINFO_Node pNode);           // insert nodeinfo ，将节点数据临时存放
    void                    insertNodeFinished();                        // 调用后，将节点插入到节点列表中
    UINT16              getNodeSize();                      // get size of node
    void                    removeAllNodes();                   // remove all nodes
    void                    deleteNodeByAddr(UINT16 addr);      //
    std::string             getName();
    void                    setName(std::string name);
    pINFO_Node      getCurNode();
    pINFO_Node      getTmpNode();
    pINFO_Node      getNextNode();
    void                setCurNode(UINT16 curAddr);
    void                increaseUnReplyNum(int inum=1);             //增加当前节点未返回的请求数
    int                 getUnReplyNum();                                        //返回当前节点未返回的请求数
    void                resetUnReplyNum();                                  //重置当前节点的未返回的请求数
    void                setCurOperatorType(OperatorType oType);
    OperatorType    getCurOperatorType();
    bool                isExistNode(UINT16 addr);

private:
    // TcpConnectionPtr     m_pConn;
    pINFO_Node          m_pCurNode;
    pINFO_Node               m_pTmpNode;             //临时节点对象，调用 insertFinished后插入到节点列表中
    // std::vector<INFO_Node> m_vNodes;
    std::map<UINT16, pINFO_Node> m_mNodesInfo;
    std::vector<UINT16> m_vNodeAddrs;
    std::string         m_strName;
    UINT16              m_curNodeAddr;
    OperatorType        m_CurOperatortype;
    int                 m_curIndex;

    // addr unreplyNum

};

Gateway::Gateway()
{
    // m_pConn = NULL;
    m_pCurNode = NULL;
    m_strName = "";
    m_curNodeAddr=0x0000;
    m_CurOperatortype = REGISTER_NODE;
    m_curIndex = 0;
}

bool Gateway::isExistNode(UINT16 addr)
{
    std::map<UINT16,pINFO_Node>::iterator it;
    it = m_mNodesInfo.find(addr);
    if (it != m_mNodesInfo.end())
    {
        return true;
    }
    return false;
}

OperatorType Gateway::getCurOperatorType()
{
    return m_CurOperatortype;
}

void Gateway::setCurOperatorType(OperatorType oType)
{
    m_CurOperatortype = oType;
}

void Gateway::increaseUnReplyNum(int inum = 1)
{
    m_pCurNode->unReplyNum = m_pCurNode->unReplyNum + inum;
}


int Gateway::getUnReplyNum()
{
    return m_pCurNode->unReplyNum;
}

void Gateway::resetUnReplyNum()
{
    if(m_pCurNode == NULL)
    {
        return;
    }
    m_pCurNode->unReplyNum =0;
}

pINFO_Node Gateway::getCurNode()
{
    if (m_pCurNode == NULL)
    {
        m_pCurNode = m_mNodesInfo[m_vNodeAddrs[0]];
    }
    return m_pCurNode;
}

pINFO_Node Gateway::getTmpNode()
{
    if (m_pTmpNode != NULL)
    {
        return m_pTmpNode;
    }
    return NULL;
}

pINFO_Node Gateway::getNextNode()
{
    m_curIndex++;
    LOG_INFO << "m_curIndex: " << m_curIndex
             << " m_mNodesInfo.size(), " << m_mNodesInfo.size();

    // m_curIndex%(m_mNodesInfo.size());
    m_curIndex = m_curIndex%(m_mNodesInfo.size());

    LOG_INFO << "++++++++++++++++ m_vNodeAddrs[" << m_curIndex << "]: " << m_vNodeAddrs[m_curIndex];
    m_pCurNode = m_mNodesInfo[m_vNodeAddrs[m_curIndex]];
    return m_pCurNode;
}

void Gateway::setCurNode(UINT16 curAddr)
{
    m_pCurNode = m_mNodesInfo.find(curAddr)->second;
}

pINFO_Node Gateway:: getNodeByAddr(UINT16 addr)
{
    std::map<UINT16,pINFO_Node>::iterator it;
    it = m_mNodesInfo.find(addr);
    if (it != m_mNodesInfo.end())
    {
        return it->second;
    }

    return NULL;
}

void Gateway::insertNode(pINFO_Node pNode)
{

    if(m_pTmpNode == NULL)
    {
        m_pTmpNode = new INFO_Node();
    }

    if(pNode != NULL )
    {
        m_pTmpNode->addr = pNode->addr;
        m_pTmpNode->unReplyNum = pNode->unReplyNum;
        //m_pTmpNode->state =
        LOG_INFO << " m_pTmpNode->addr: " <<  m_pTmpNode->addr;
    }
    else
    {
        // WARN

    }


}

void Gateway::insertNodeFinished()
{

    if(m_pTmpNode != NULL)
    {
        m_mNodesInfo[m_pTmpNode->addr] = m_pTmpNode;
        m_vNodeAddrs.push_back(m_pTmpNode->addr);
    }
    else
    {
        //warn....here
    }
}

UINT16 Gateway::getNodeSize()
{
    return (UINT16)m_mNodesInfo.size();
}

void Gateway::removeAllNodes()
{
    m_mNodesInfo.clear();
    m_vNodeAddrs.clear();
}

void Gateway::deleteNodeByAddr(UINT16 addr)
{
    m_mNodesInfo.erase(addr);

    for (int i = 0; i < m_vNodeAddrs.size(); ++i)
    {
        if (m_vNodeAddrs[i] == addr)
        {
            m_vNodeAddrs.erase(m_vNodeAddrs.begin()+i);
            LOG_INFO << "^^^^^^^^^^^^^^^^ node," << addr << " was deleted!";
        }
    }
}

std::string Gateway::getName()
{
    return m_strName;
}

void Gateway::setName(std::string name)
{
    m_strName = name;
}

#endif