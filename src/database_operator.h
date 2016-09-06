#ifndef DATABASE_OPERATOR_H
#define DATABASE_OPERATOR_H

#include <mysql++.h>
#include <vector>
#include <muduo/base/StringPiece.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include "preDef.h"


#define DATABASE_SERVER_IP "127.0.0.1"
#define DATABASE_NAME  "jacdb"

using namespace std;

typedef struct 
{
    UINT16  task_type;                      //different task ,different select cause;
    UINT8    operator_type;             // insert ,select ,update ,and so on
    char    content[STRING_MAXLEN] ;        // detailed content
}DatabaseOperatorTask, *pDatabaseOperatorTask; 


class DatabaseOperator
{

public:
    
    DatabaseOperator():conn_(false) {     }


    int Init();
    
    vector<UINT16> GetNodesOfGateway (string ipaddr );
    bool DeleteNodeofGateway(string ipaddr, UINT16 node);
    bool InsertNodeOfGateway(string ipaddr, UINT16 node);
    bool UpdateNodesOfGateway(string ipaddr, string name);
    string GetNameOfGateWay(string ipaddr);


private:
    mysqlpp::Connection conn_;   


public:
    typedef std::vector<DatabaseOperatorTask> DatabaseOperatorTaskList;
    DatabaseOperatorTaskList                        tasks_;                // database task list
    mutable muduo::MutexLock                        task_list_mutex_;                               //
    


};

#endif
