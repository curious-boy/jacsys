#include "database_operator.h"
#include <sstream>

// create database connect
int DatabaseOperator::Init()
{
    conn_.set_option(new mysqlpp::SetCharsetNameOption("utf8"));

    if(conn_.connect(DATABASE_NAME,DATABASE_SERVER_IP,"root","111111"))
    {
        LOG_INFO <<"DB Connection success..";
    }
    else
    {
        LOG_ERROR <<"DB Connection failed: " << conn_.error();
        return -1;
    }

    return 0;
}

bool DatabaseOperator::ExecTasks()
{
    if(GetTaskList()==false)
    {
        return false;
    }

    int ilen=tasks_beExec_.size();
    UINT16 taskType;
    UINT8  operatorType;

    for(int i=0; i<ilen; i++)
    {
        taskType = tasks_beExec_[i].task_type;
        operatorType = tasks_beExec_[i].operator_type;
        LOG_DEBUG << "task content: " << tasks_beExec_[i].content;

        // 0 insert 1 update 2 other

        switch(operatorType)
        {
            case 0:
                {
                    mysqlpp::Query query = conn_.query(tasks_beExec_[i].content.c_str());
                    query.exec();
                }
                break;
            case 1:
                {
                    mysqlpp::Query query = conn_.query(tasks_beExec_[i].content.c_str());
                    query.exec();

                }
                break;
            default:
                {
                    LOG_DEBUG<<"unknown sql operator";
                }
                break;
        }
    }

}

bool DatabaseOperator::GetTaskList()
{
    MutexLockGuard lock(task_list_mutex_);
    if(tasks_.size() == 0)
    {
        return false;
    }
    tasks_beExec_.swap(tasks_);
    tasks_.clear();
    return true;

}

void DatabaseOperator::AddTask(DatabaseOperatorTask task)
{
    MutexLockGuard lock(task_list_mutex_);
    tasks_.push_back(task);
}

std::vector<INFO_Node>  DatabaseOperator::GetNodesOfGateway (string ipaddr )
{
    std::vector<INFO_Node> tvNodes;
    string strsql = "select node_zig_addr,machine_id from node_register_info where gateway_ip='" + ipaddr + "'";
    LOG_DEBUG << "GetNodesOfGateway strsql, " << strsql;

    mysqlpp::Query query = conn_.query(strsql.c_str());
    if (mysqlpp::StoreQueryResult res = query.store())
    {

        mysqlpp::StoreQueryResult::const_iterator it;
        INFO_Node tmpNode;
        for (it = res.begin(); it != res.end(); ++it)
        {
            mysqlpp::Row row = *it;
            tmpNode.macId.clear();
            tmpNode.macId.insert(0,row[1]);
            tmpNode.addr = row[0];
            tvNodes.push_back(tmpNode);
        }
    }
    else
    {
        LOG_DEBUG << "There is no node of  ipaddr. " ;
    }

    return tvNodes;
}

UINT16 DatabaseOperator::GetZigAddrOfGateway(string ipaddr)
{
    UINT16 zigAddr;
    string strsql = "select gateway_zig_addr from node_register_info where gateway_ip='" + ipaddr + "'";
    LOG_DEBUG << "GetZigAddrOfGateway strsql, " << strsql;

    mysqlpp::Query query = conn_.query(strsql.c_str());
    if (mysqlpp::StoreQueryResult res = query.store())
    {

        mysqlpp::StoreQueryResult::const_iterator it;
        for (it = res.begin(); it != res.end(); ++it)
        {
            mysqlpp::Row row = *it;

            zigAddr = row[0];
            return zigAddr;
        }
    }
    else
    {
        LOG_DEBUG << "There is no node of  ipaddr. " ;
    }

    return 0;
}

bool  DatabaseOperator::DeleteNodeOfGateway(string ipaddr, UINT16 node)
{
    std::vector<UINT16> tvNodes;

    std::ostringstream ostrsql;
    if(node == 0)
    {
        ostrsql << "delete from node_register_info where gateway_ip='" << ipaddr << "'";
    }
    else
    {
        ostrsql << "delete from node_register_info where gateway_ip='" << ipaddr<< "' and node_zig_addr=" << node;
    }
    std::cout<<"DeleteNodeOfGateway: "<< ostrsql.str()<<std::endl;
    mysqlpp::Query query = conn_.query(ostrsql.str().c_str());
    query.exec();

    return true;
}

bool  DatabaseOperator::DeleteNodesOfGateway(string ipaddr)
{
    std::vector<UINT16> tvNodes;

    std::ostringstream ostrsql;

    ostrsql << "delete from node_register_info where gateway_ip='" << ipaddr<<"'";

    mysqlpp::Query query = conn_.query(ostrsql.str().c_str());
    query.exec();

    return true;
}

bool  DatabaseOperator::InsertNodeOfGateway(string ipaddr, UINT16 g_zig, UINT16 node)
{
    //插入之前查询是否有相同节点存在
    std::vector<UINT16> tvNodes;
    std::ostringstream ostrsql;
    ostrsql << "insert into node_register_info (gateway_name, gateway_ip,gateway_zig_addr,node_zig_addr) VALUES ('','"
            << ipaddr << "' ,"<< g_zig << ","  << node << ")";

    mysqlpp::Query query = conn_.query(ostrsql.str().c_str());
    LOG_DEBUG<< "InsertNodeOfGateway, sql: " << ostrsql.str().c_str();

    if (mysqlpp::StoreQueryResult res = query.store())
    {

        mysqlpp::StoreQueryResult::const_iterator it;
        for (it = res.begin(); it != res.end(); ++it)
        {
            mysqlpp::Row row = *it;
            //LOG_INFO << "node addr = " << row[0] ;
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


bool  DatabaseOperator::IsRecordExist(std::string sql)
{
    //查询数据是否存在

    mysqlpp::Query query = conn_.query(sql);
    if (mysqlpp::StoreQueryResult res = query.store())
    {
        if (res.num_rows()>0)
        {
            return true;
        }
    }
    else
    {
        return false;
    }

    return false;

}


string  DatabaseOperator::GetNameOfGateWay(string ipaddr)
{
    return "";
}

