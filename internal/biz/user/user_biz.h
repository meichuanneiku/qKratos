#ifndef USERBIZ_H
#define USERBIZ_H

#include <QObject>
#include <QJsonObject>
#include <QSqlQuery>
#include <QCryptographicHash>

#include "../../data/user/user_repo.h"



class UserBiz: public QObject
{
    Q_OBJECT
public:
    explicit UserBiz(QObject* parent = nullptr);

    int CreateUser(const QString& name);
    QJsonObject GetUser(const int& systemId, QString condition);
    QJsonObject FindById(const int &systemId, QString id);

    bool DeleteUser(int id);

    QJsonObject UserLogin(const int& systemId, const QString& name,const QString& password);

signals:
    void userCreated(const QJsonObject& user);
};

#endif // USERBIZ_H
