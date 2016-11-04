#ifndef DATABASE_OPERATOR_H
#define DATABASE_OPERATOR_H

#include <mysql++.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

//#include <muduo/base/StringPiece.h>
#include <muduo/base/AsyncLogging.h>
#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include "MsgTypeDef.h"
#include "StuDef.h"


#define DATABASE_SERVER_IP "127.0.0.1"
#define DATABASE_NAME  "jacdb"

//using namespace std;
using namespace muduo;

typedef struct
{
    UINT16  task_type;                      //different task ,different select cause;
    UINT8    operator_type;             // insert ,select ,update ,and so on
    string    content ;        // detailed content
}DatabaseOperatorTask, *pDatabaseOperatorTask;


class DatabaseOperator
{

public:

    DatabaseOperator():conn_(false) {     }


    int Init();

    bool reConnect();

    std::vector<INFO_Node> GetNodesOfGateway (string ipaddr );
    UINT16 GetZigAddrOfGateway(string ipaddr);
    bool DeleteNodeOfGateway(string ipaddr, UINT16 node);
    bool DeleteNodesOfGateway(string ipaddr);


    bool InsertNodeOfGateway(string ipaddr,UINT16 g_zig, UINT16 node);

    bool UpdateNodesOfGateway(string ipaddr, string name);
    string GetNameOfGateWay(string ipaddr);

    bool ExecTasks();
    void AddTask(DatabaseOperatorTask task);



    private:
        bool GetTaskList();



private:
    mysqlpp::Connection conn_;

    typedef std::vector<DatabaseOperatorTask> DatabaseOperatorTaskList;
    DatabaseOperatorTaskList                        tasks_;                // database task list
    DatabaseOperatorTaskList                        tasks_beExec_;                // database task list will be execed
    mutable MutexLock                                   task_list_mutex_;                               //
    mutable MutexLock                               conn_mutex_;                   // mutex for conn

    void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}



};

#endif
