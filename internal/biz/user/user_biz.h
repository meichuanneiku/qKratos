#ifndef USERBIZ_H
#define USERBIZ_H

#include <QObject>
#include <QJsonObject>
#include "../../data/user/user_repo.h"
class UserBiz: public QObject
{
    Q_OBJECT
public:
    explicit UserBiz(QObject* parent = nullptr);

    int CreateUser(const QString& name);
    QJsonObject GetUser(int id);
    void DeleteUser(int id);

signals:
    void userCreated(const QJsonObject& user);
};

#endif // USERBIZ_H
