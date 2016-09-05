#ifndef DATABASE_OPERATOR_H
#define DATABASE_OPERATOR_H
#include <mysql++.h>
#include <muduo/base/Mutex.h>


typedef struct 
{
    UINT16  task_type;                      //different task ,different select cause;
    UINT8    operator_type;             // insert ,select ,update ,and so on
    char    content[STRING_MAXLEN] ;        // detailed content
}DatabaseOperatorTask, *pDatabaseOperatorTask; 


class DatabaseOperator
{
public:
    DatabaseOperator();
    ~DatabaseOperator();

    int Init();


private:
    mysqlpp::Connection conn_;
    
    typedef std::vector<DatabaseOperatorTask> DatabaseOperatorTaskList;
    static DatabaseOperatorTaskList     tasks_;                // database task list
    mutable MutexLock                       task_list_mutex_;                               //

private:
    

}

#endif
