// the header of class gateway





class gateway
{
public:
	gateway();
	~gateway();
	



private:
	string m_strGatewayId;
	TcpConnectionPtr m_pConn;
	UINT16 m_iSrcAddr;
	// std::vector<INFO_Node> m_vNodes;
	std::std::map<UINT16, UINT8> m_mNodesInfo;
	// addr unreplyNum

};