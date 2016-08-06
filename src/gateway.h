// the header of class gateway
#ifndef GATEWAY_H
#define GATEWAY_H

#include "StuDef.h"
#include <string>
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
	void		setCurNode(UINT16 curAddr);
	void 		setCurOperatorType(OperatorType oType);
	OperatorType getCurOperatorType();


private:
	// TcpConnectionPtr 	m_pConn;
	pINFO_Node 			m_pCurNode;
	// std::vector<INFO_Node> m_vNodes;
	std::map<UINT16, pINFO_Node> m_mNodesInfo;
	std::string 		m_strName;
	UINT16				m_curNodeAddr;
	OperatorType		m_CurOperatortype;

	// addr unreplyNum

};

gateway::gateway()
{
	// m_pConn = NULL;
	m_pCurNode = NULL;
	m_strName = "";
	m_curNodeAddr=0x0000;
	m_CurOperatortype = MODIFY_DEST_NODE;
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
		m_mNodesInfo[pNode->addr] = pNode;
	}
}

UINT16 gateway::getNodeSize()
{
	return (UINT16)m_mNodesInfo.size();
}

void gateway::removeAllNodes()
{
	m_mNodesInfo.clear();
}

void gateway::deleteNodeByAddr(UINT16 addr)
{
	m_mNodesInfo.erase(addr);
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