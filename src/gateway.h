// the header of class Gateway
#ifndef GATEWAY_H
#define GATEWAY_H

#include "StuDef.h"
#include <string>
#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
// using namespace std;

using namespace muduo;

typedef enum
{
    REGISTER_NODE,
    REGISTER_FINISH,
    LOGOUT_NODE,
    LOGOUT_FINISH,
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
    UINT16              getNodeSize();                      // get size of node
    void                resetCurNode();                         // reset m_curIndex to 0
    void                removeAllNodes();                   // remove all nodes
    void                deleteNodeByAddr(UINT16 addr);      //
    string              getName();
    void                setName(string name);
    pINFO_Node          getCurNode();
    void                updateNodeByAddr(UINT16 addr,pINFO_Node pnode);
    pINFO_Node          getNextNode();
    void                setCurNode(UINT16 curAddr);
    void                increaseUnReplyNum(int inum=1);             //增加当前节点未返回的请求数
    int                 getUnReplyNum();                                        //返回当前节点未返回的请求数
    void                resetUnReplyNum(UINT16 addr=0);                                  //重置当前节点的未返回的请求数
    void                setCurOperatorType(OperatorType oType);
    OperatorType        getCurOperatorType();
    bool                isExistNode(UINT16 addr);
    void                setIp(string ip);
    string              getIp();



private:
    std::vector<pINFO_Node> m_vNodesInfo;
    string         m_strName;
    string         m_strIP;        // ip address
    UINT16              m_curNodeAddr;
    OperatorType        m_CurOperatortype;
    int                     m_curIndex;

    // addr unreplyNum

};

Gateway::Gateway()
{
    // m_pConn = NULL;
    m_strName = "";
    m_strIP = "";
    m_CurOperatortype = SEND_MESSAGE;
    m_curIndex = 0;
}

bool Gateway::isExistNode(UINT16 addr)
{
    for(int i=0; i<m_vNodesInfo.size(); i++)
    {
        if(m_vNodesInfo[i]->addr == addr)
        {
            return true;
        }
    }
    return false;
}

void Gateway::setIp(string ip)
{
    m_strIP = ip;
}

string Gateway::getIp()
{
    return m_strIP;
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
    m_vNodesInfo[m_curIndex]->unReplyNum= m_vNodesInfo[m_curIndex]->unReplyNum + inum;
}


int Gateway::getUnReplyNum()
{
    return m_vNodesInfo[m_curIndex]->unReplyNum;
}

void Gateway::resetUnReplyNum(UINT16 addr=0)
{
    LOG_INFO<<"resetUnReplyNum, addr:="<< addr;
    if(addr==0)
    {
        if(m_vNodesInfo.size()==0)
        {
            return;
        }
        m_vNodesInfo[m_curIndex]->unReplyNum =0;
    }

    int len=m_vNodesInfo.size();
    for(int i=0; i<len; i++)
    {
        if(m_vNodesInfo[i]->addr==addr)
        {
            m_vNodesInfo[i]->unReplyNum=0;
        }
    }
}



pINFO_Node Gateway::getCurNode()
{
    if(m_vNodesInfo.size() < 0 )
    {
        return NULL;
    }
    return m_vNodesInfo[m_curIndex];
}

pINFO_Node Gateway::getNextNode()
{
    m_curIndex++;
    LOG_INFO << "m_curIndex: " << m_curIndex
             << " m_vNodesInfo.size(), " << m_vNodesInfo.size();

    m_curIndex = m_curIndex%(m_vNodesInfo.size());

    return m_vNodesInfo[m_curIndex];
}

void Gateway::setCurNode(UINT16 curAddr)
{
    for(int i=0; i<m_vNodesInfo.size(); i++)
    {
        if(m_vNodesInfo[i]->addr == curAddr)
        {
            m_curIndex = i;
        }
    }
}

pINFO_Node Gateway::getNodeByAddr(UINT16 addr)
{
    for(int i=0; i<m_vNodesInfo.size(); i++)
    {
        if(m_vNodesInfo[i]->addr == addr)
        {
            return m_vNodesInfo[i];
        }
    }
    return NULL;
}

void Gateway::updateNodeByAddr(UINT16 addr,pINFO_Node pnode)
{
    for(int i=0; i<m_vNodesInfo.size(); i++)
    {
        if(m_vNodesInfo[i]->addr == addr)
        {
            m_vNodesInfo[i] = pnode;
        }
    }
}

void Gateway::insertNode(pINFO_Node pNode)
{
    if(!isExistNode(pNode->addr) )
    {
        if(pNode != NULL )
        {
            m_vNodesInfo.push_back(pNode);
            LOG_INFO << " pNode->addr: " <<  pNode->addr;
        }
    }
}

UINT16 Gateway::getNodeSize()
{
    return (UINT16)m_vNodesInfo.size();
}

void Gateway::resetCurNode()
{
    m_curIndex=0;
}

void Gateway::removeAllNodes()
{
    m_vNodesInfo.clear();
}

void Gateway::deleteNodeByAddr(UINT16 addr)
{
    for(int i=0; i<m_vNodesInfo.size(); i++)
    {
        if(m_vNodesInfo[i]->addr == addr)
        {
            delete m_vNodesInfo[i];
            m_vNodesInfo[i]=NULL;
            m_vNodesInfo.erase(m_vNodesInfo.begin()+i);
        }
    }
}

string Gateway::getName()
{
    return m_strName;
}

void Gateway::setName(string name)
{
    m_strName = name;
}

#endif
