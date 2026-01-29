#ifndef GLOBAL_H
#define GLOBAL_H

#include <QSqlQuery>
#include <QJsonObject>
#include <QString>
#include "../error/error_code.h"
using namespace qKratos::Error;

struct QueryResult {
    QSqlQuery query;
    ErrorCode errorCode = Success;
    QString errorMessage;

    bool isSuccess() const {
        return errorCode == Success;
    }
};


struct JsonObjectResult {
    QJsonObject data;
    ErrorCode errorCode = Success;
    QString errorMessage;

    bool isSuccess() const {
        return errorCode == Success;
    }
};


#endif // GLOBAL_H
