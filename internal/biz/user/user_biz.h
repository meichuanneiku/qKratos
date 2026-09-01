#ifndef USERBIZ_H
#define USERBIZ_H

#include <QJsonObject>
#include <QJsonArray>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCryptographicHash>
#include <QtMath>


#include "../../data/user/user_repo.h"
#include "../../pkg/global/global.h"


class UserBiz
{
public:
    UserBiz() = default;

    int CreateUser(const int& systemId, const QString& name);
    QJsonObject GetUser(const int& systemId, QString condition);
    QJsonObject FindById(const int &systemId, QString id);
    QJsonObject ListUsers(const int& systemId, const QString& page);

    bool DeleteUser(const int& systemId, int id);
    bool UpdateUser(const int& systemId, const QJsonObject& user);

    JsonObjectResult UserLogin(const int& systemId, const QString& name,const QString& password);

    JsonObjectResult UserList(const int& systemId, const QString& fxtid,const QString& jsid,
                              const QString& zxzt, const QString& yhmc,
                              const QString& dlcsCondition, const int& dlcs,
                              const int& page, const int& pageSize);


    JsonObjectResult deviceParamPush(const QJsonObject &doc);
};

#endif // USERBIZ_H
