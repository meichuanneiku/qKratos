#ifndef USERBIZ_H
#define USERBIZ_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QCryptographicHash>
#include <QtMath>


#include "../../data/user/user_repo.h"
#include "../../pkg/global/global.h"


class UserBiz: public QObject
{
    Q_OBJECT
public:
    explicit UserBiz(QObject* parent = nullptr);

    int CreateUser(const QString& name);
    QJsonObject GetUser(const int& systemId, QString condition);
    QJsonObject FindById(const int &systemId, QString id);

    bool DeleteUser(int id);

    JsonObjectResult UserLogin(const int& systemId, const QString& name,const QString& password);

    JsonObjectResult UserList(const int& systemId, const QString& fxtid,const QString& jsid,
                              const QString& zxzt, const QString& yhmc,
                              const QString& dlcsCondition, const int& dlcs,
                              const int& page, const int& pageSize);


    JsonObjectResult deviceParamPush(const QJsonObject &doc);



signals:
    void userCreated(const QJsonObject& user);
};

#endif // USERBIZ_H
