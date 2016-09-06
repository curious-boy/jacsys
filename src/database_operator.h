#ifndef DATABASE_OPERATOR_H
#define DATABASE_OPERATOR_H

#include <mysql++.h>
#include <muduo/base/Mutex.h>
#include "preDef.h"


#define DATABASE_SERVER_IP "127.0.0.1"


typedef struct 
{
    UINT16  task_type;                      //different task ,different select cause;
    UINT8    operator_type;             // insert ,select ,update ,and so on
    char    content[STRING_MAXLEN] ;        // detailed content
}DatabaseOperatorTask, *pDatabaseOperatorTask; 


class DatabaseOperator
{

public:
    
    DatabaseOperator() {     }


    int Init();


private:
    mysqlpp::Connection conn_;
    


public:
    typedef std::vector<DatabaseOperatorTask> DatabaseOperatorTaskList;
    DatabaseOperatorTaskList                        tasks_;                // database task list
    mutable muduo::MutexLock                        task_list_mutex_;                               //


};

#endif
