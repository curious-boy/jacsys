#include "database_operator.h"
#include <sstream>

// create database connect
int DatabaseOperator::Init()
{
    conn_.set_option(new mysqlpp::SetCharsetNameOption("utf8"));

    if(conn_.connect(DATABASE_NAME,DATABASE_SERVER_IP,"root","111111"))
    {
        LOG_INFO <<"DB Connection success: ";
        mysqlpp::Query query = conn_.query("select job_id,name from employee_info");
        if (mysqlpp::StoreQueryResult res = query.store())
        {
            LOG_INFO << "We have:" ;
            mysqlpp::StoreQueryResult::const_iterator it;
            for (it = res.begin(); it != res.end(); ++it)
            {
                mysqlpp::Row row = *it;
                //LOG_INFO << row[0] << " | " << row[1] ;
		cout << '\t' << row[0] << '\t' << row[1] << endl;
            }
        }
        else
        {
            LOG_ERROR << "Failed to get item list: " << query.error() ;

        }
    }
    else
    {
        LOG_ERROR <<"DB Connection failed: " << conn_.error();
        return -1;
    }

    return 0;
}

bool DatabaseOperator::ExecTask(DatabaseOperatorTask& task)
{
    UINT16 taskType = task.task_type;
    UINT8  operatorType = task.operator_type;
    LOG_DEBUG << "task content: " << task.content;

    switch(taskType)
    {
        case MSG_REPLY|MSG_GETPRODUCTION:
        {
            // add code here


            break;
        }
        case MSG_REPLY|MSG_GETFIRMWAREINFO:
        {
            //add code here
            break;
        }

        default:
            break;
    }
}


std::vector<UINT16>  DatabaseOperator::GetNodesOfGateway (std::string ipaddr )
{
    std::vector<UINT16> tvNodes;
    mysqlpp::Query query = conn_.query("select node from node_register_info");
    if (mysqlpp::StoreQueryResult res = query.store())
    {

        mysqlpp::StoreQueryResult::const_iterator it;
        for (it = res.begin(); it != res.end(); ++it)
        {
            mysqlpp::Row row = *it;
            cout << "node addr = " << row[0] <<endl;;
            tvNodes.push_back(row[0]);
        }
    }
    else
    {
        LOG_DEBUG << "There is no node of  ipaddr. " ;
    }

    return tvNodes;
}

bool  DatabaseOperator::DeleteNodeofGateway(std::string ipaddr, UINT16 node)
{
    std::vector<UINT16> tvNodes;

    ostringstream ostrsql;
    if(node == 0)
    {
        ostrsql << "delete from node_register_info where ip=" << ipaddr;
    }
    else
    {
        ostrsql << "delete from node_register_info where ip=" << ipaddr<< " and node=" << node;
    }
    mysqlpp::Query query = conn_.query(ostrsql.str().c_str());

    return true;
}

bool  DatabaseOperator::InsertNodeOfGateway(std::string ipaddr, UINT16 node)
{
	std::vector<UINT16> tvNodes;
	ostringstream ostrsql;
	ostrsql << "select node from node_register_info where ip=" << ipaddr << " and node=" << node;
    mysqlpp::Query query = conn_.query(ostrsql.str().c_str());
	
    if (mysqlpp::StoreQueryResult res = query.store())
    {

        mysqlpp::StoreQueryResult::const_iterator it;
        for (it = res.begin(); it != res.end(); ++it)
        {
            mysqlpp::Row row = *it;
            cout << "node addr = " << row[0] <<endl;;
            tvNodes.push_back(row[0]);
        }
    }
    else
    {
        LOG_DEBUG << "There is no node of  ipaddr. " ;
    }
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

