// the header of class gateway
#ifndef GATEWAY_H
#define GATEWAY_H

#include "StuDef.h"
#include <string>
#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
// using namespace std;


typedef enum 
{
	MODIFY_DEST_NODE,
	SEND_MESSAGE
	
}OperatorType;

class gateway
{
public:
	gateway();
	~gateway();
	

public:
	pINFO_Node 	getNodeByAddr(UINT16 addr);			// get nodeinfo by addr	
	void 		insertNode(pINFO_Node pNode);			// insert nodeinfo 
	UINT16 		getNodeSize();						// get size of node 	
	void 		removeAllNodes();					// remove all nodes
	void 		deleteNodeByAddr(UINT16 addr);		// 
	std::string		getName();
	void 		setName(std::string name);
	pINFO_Node	getCurNode();
	pINFO_Node  getNextNode();
	void		setCurNode(UINT16 curAddr);
	void 		setCurOperatorType(OperatorType oType);
	OperatorType getCurOperatorType();
	bool 		isExistNode(UINT16 addr);

private:
	// TcpConnectionPtr 	m_pConn;
	pINFO_Node 			m_pCurNode;
	// std::vector<INFO_Node> m_vNodes;
	std::map<UINT16, pINFO_Node> m_mNodesInfo;
	std::vector<UINT16> m_vNodeAddrs;
	std::string 		m_strName;
	UINT16				m_curNodeAddr;
	OperatorType		m_CurOperatortype;
	int 				m_curIndex;

	// addr unreplyNum

};

gateway::gateway()
{
	// m_pConn = NULL;
	m_pCurNode = NULL;
	m_strName = "";
	m_curNodeAddr=0x0000;
	m_CurOperatortype = MODIFY_DEST_NODE;
	m_curIndex = 0;
}

bool gateway::isExistNode(UINT16 addr)
{
	std::map<UINT16,pINFO_Node>::iterator it;
	it = m_mNodesInfo.find(addr);
	if (it != m_mNodesInfo.end())
	{
		return true;
	}
	return false;
}

OperatorType gateway::getCurOperatorType()
{
	return m_CurOperatortype;
}

void gateway::setCurOperatorType(OperatorType oType)
{
	m_CurOperatortype = oType;
}

pINFO_Node gateway::getCurNode()
{
	if (m_pCurNode == NULL)
	{
		m_pCurNode = m_mNodesInfo[m_vNodeAddrs[0]];
	}
	return m_pCurNode;
}

pINFO_Node gateway::getNextNode()
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

void gateway::setCurNode(UINT16 curAddr)
{
	m_pCurNode = m_mNodesInfo.find(curAddr)->second;
}

pINFO_Node gateway:: getNodeByAddr(UINT16 addr)
{
	std::map<UINT16,pINFO_Node>::iterator it;
	it = m_mNodesInfo.find(addr);
	if (it != m_mNodesInfo.end())
	{
		return it->second;
	}

	return NULL;
}

void gateway::insertNode(pINFO_Node pNode)
{
	if (pNode != NULL)
	{
		LOG_INFO << "gateway::insertNode 01 ";
		m_mNodesInfo[pNode->addr] = pNode;
		m_vNodeAddrs.push_back(pNode->addr);
		LOG_INFO << "gateway::insertNode 02 ";
	}
}

UINT16 gateway::getNodeSize()
{
	return (UINT16)m_mNodesInfo.size();
}

void gateway::removeAllNodes()
{
	m_mNodesInfo.clear();
	m_vNodeAddrs.clear();
}

void gateway::deleteNodeByAddr(UINT16 addr)
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

std::string gateway::getName()
{
	return m_strName;
}

void gateway::setName(std::string name)
{
	m_strName = name;
}

#endif