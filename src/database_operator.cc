#include "database_operator.h"

// create database connect
int DatabaseOperator::Init()
{
    conn.set_option(new mysqlpp::SetCharsetNameOption("utf8"));

    if(conn_.connect(DATABASE_NAME,DATABASE_SERVER_IP,"root","111111"))
    {
        LOG_INFO <<"DB Connection success: ";
    }
    else
    {
        LOG_ERROR <<"DB Connection failed: " << conn.error();
    }

    return 0;
}


std::vector<UINT16>  DatabaseOperator::GetNodesOfGateway (string ipaddr )
{
    std::vector<UINT16> tvNodes;
    mysqlpp::Query query = conn.query("select node from node_register_info");
    if (mysqlpp::StoreQueryResult res = query.store())
    {

        mysqlpp::StoreQueryResult::const_iterator it;
        for (it = res.begin(); it != res.end(); ++it)
        {
            mysqlpp::Row row = *it;
            LOG_DEBUG << "node addr = " << row[0];
            tvNodes.push_back(row[0]);
        }
    }
    else
    {
        LOG_DEBUG << "There is no node of  ipaddr. " ;
    }

    return tvNodes;
}

bool  DatabaseOperator::DeleteNodeofGateway(string ipaddr, UINT16 node)
{
	//mysqlpp::Query query = conn.query("delete from node_register_info where ");
    return true;
}

bool  DatabaseOperator::InsertNodeOfGateway(string ipaddr, UINT16 node)
{
    return true;
}

bool  DatabaseOperator::UpdateNodesOfGateway(string ipaddr, string name)
{
    return true;
}


string  DatabaseOperator::GetNameOfGateWay(string ipaddr)
{
    return "";
}

