#include "database_operator.h"
#include <sstream>
#include "XMLConfig.h"

extern XMLConfig g_config;

// create database connect
int DatabaseOperator::Init()
{
    MutexLockGuard lock(conn_mutex_);

    conn_.set_option(new mysqlpp::SetCharsetNameOption("utf8"));

    if(conn_.connect(g_config.dbName.c_str(),g_config.dbServerIp.c_str(),g_config.userName.c_str(),g_config.password.c_str()))
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

bool DatabaseOperator::reConnect()
{
    MutexLockGuard lock(conn_mutex_);

    if( conn_.connected() )
    {
        return true;
    }

    conn_.disconnect();

    if(conn_.connect(g_config.dbName.c_str(),g_config.dbServerIp.c_str(),g_config.userName.c_str(),g_config.password.c_str()))
    {
        LOG_INFO <<"DB Connection success..";
        return true;
    }
    else
    {
        LOG_ERROR <<"DB Connection failed: " << conn_.error();
        return false;
    }
}

void DatabaseOperator::Ping()
{
	MutexLockGuard lock(conn_mutex_);

    if( conn_.connected() )
    {
        conn_.ping();
    }
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

    reConnect();

    MutexLockGuard lock(conn_mutex_);

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
                    if(!query.exec())
                    {
                        LOG_WARN<<tasks_beExec_[i].content.c_str()<<" insert_task failed,errnum:= "<<query.errnum()<<",error msg: "<<query.error();
                    }
                }
                break;
            case 1:
                {
                    mysqlpp::Query query = conn_.query(tasks_beExec_[i].content.c_str());
                    if(!query.exec())
                    {
                        LOG_WARN<<tasks_beExec_[i].content.c_str()<<" update_task failed,errnum:= "<<query.errnum()<<",error msg: "<<query.error();
                    }
                }
                break;
            case 2:
                {
                    std::vector<std::string> sqlVector=split(tasks_beExec_[i].content.c_str(),';');
                    std::string selectsql= sqlVector[0];
                    std::string updatesql= sqlVector[1];
                    std::string insertsql= sqlVector[2];

                    bool result=false;
                    mysqlpp::Query query = conn_.query(selectsql);
    				if (mysqlpp::StoreQueryResult res = query.store())
    				{
        				if (res.num_rows()>0)
        				{
            				result= true;
        				}
    				}
    				else
    				{
        				result= false;
    				}
    				if(result)
    				{
    					query = conn_.query(updatesql);
                    	if(!query.exec())
                    	{
                        	LOG_WARN<<tasks_beExec_[i].content.c_str()<<" update_task failed,errnum:= "<<query.errnum()<<",error msg: "<<query.error();
                    	}
    				}
    				else
    				{
    					query = conn_.query(insertsql);
                    	if(!query.exec())
                    	{
                        	LOG_WARN<<tasks_beExec_[i].content.c_str()<<" insert_task failed,errnum:= "<<query.errnum()<<",error msg: "<<query.error();
                    	}
    				}
    				sqlVector.clear();

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

    reConnect();

    MutexLockGuard lock(conn_mutex_);

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

bool  DatabaseOperator::IsNodeExist (std::string machineId,std::string gate_addr,std::string node_addr )
{
    std::string strsql =
        "select gateway_ip from node_register_info where machine_id='" + machineId + "'"+" and "
        + "gateway_zig_addr=" + gate_addr + " and "
        + "node_zig_addr=" + node_addr;
    LOG_DEBUG << "IsNodeExist strsql, " << strsql;

    reConnect();

    MutexLockGuard lock(conn_mutex_);

    mysqlpp::Query query = conn_.query(strsql.c_str());
    if (mysqlpp::StoreQueryResult res = query.store())
    {
        if(res.num_rows()>0)
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

bool  DatabaseOperator::IsRecordExist (std::string strsql )
{
    reConnect();

    MutexLockGuard lock(conn_mutex_);

    mysqlpp::Query query = conn_.query(strsql.c_str());
    if (mysqlpp::StoreQueryResult res = query.store())
    {
        if(res.num_rows()>0)
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

std::string  DatabaseOperator::ExecuteScaler (std::string strsql )
{
    reConnect();

    MutexLockGuard lock(conn_mutex_);

    mysqlpp::Query query = conn_.query(strsql.c_str());
    if (mysqlpp::StoreQueryResult res = query.store())
    {
        if(res.num_rows()>0)
        {
            mysqlpp::StoreQueryResult::const_iterator it;
        	for (it = res.begin(); it != res.end(); ++it)
        	{
           	 	mysqlpp::Row row = *it;
            	std::string str(row[0]);
            	return str;
        	}
        }
    }
    else
    {
        return "";
    }
    
    return "";

}




UINT16 DatabaseOperator::GetZigAddrOfGateway(string ipaddr)
{
    UINT16 zigAddr;
    string strsql = "select gateway_zig_addr from node_register_info where gateway_ip='" + ipaddr + "'";
    LOG_DEBUG << "GetZigAddrOfGateway strsql, " << strsql;

    reConnect();

    MutexLockGuard lock(conn_mutex_);

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
        ostrsql << "delete from node_register_info where gateway_ip='" << ipaddr << "' limit 500 ";
    }
    else
    {
        ostrsql << "delete from node_register_info where gateway_ip='" << ipaddr<< "' and node_zig_addr=" << node<<" limit 1";
    }
    std::cout<<"DeleteNodeOfGateway: "<< ostrsql.str()<<std::endl;

    reConnect();

    MutexLockGuard lock(conn_mutex_);

    mysqlpp::Query query = conn_.query(ostrsql.str().c_str());
    query.exec();

    return true;
}

bool  DatabaseOperator::DeleteNodesOfGateway(string ipaddr)
{
    std::vector<UINT16> tvNodes;

    std::ostringstream ostrsql;

    ostrsql << "delete from node_register_info where gateway_ip='" << ipaddr<<"' limit 500";

    reConnect();

    MutexLockGuard lock(conn_mutex_);

    mysqlpp::Query query = conn_.query(ostrsql.str().c_str());
    query.exec();

    return true;
}

bool  DatabaseOperator::InsertNodeOfGateway(string ipaddr, UINT16 g_zig, UINT16 node)
{
    //插入之前查询是否有相同节点存在
    std::vector<UINT16> tvNodes;
    std::ostringstream ostrsql;

    ostrsql.str("");
    ostrsql << "insert into node_register_info (gateway_name, gateway_ip,gateway_zig_addr,node_zig_addr) VALUES ('','"
            << ipaddr << "' ,"<< g_zig << ","  << node << ")";

    reConnect();

    MutexLockGuard lock(conn_mutex_);

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




string  DatabaseOperator::GetNameOfGateWay(string ipaddr)
{
    return "";
}

