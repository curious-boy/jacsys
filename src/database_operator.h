#ifndef DATABASE_OPERATOR_H
#define DATABASE_OPERATOR_H
#include <mysql++.h>

class DatabaseOperator
{
public:
    DatabaseOperator();
    ~DatabaseOperator();



private:
    mysqlpp::Connection conn_;

    

}

#endif
