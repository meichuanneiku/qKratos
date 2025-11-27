#include "user_biz.h"

#include <QDateTime>

UserBiz::UserBiz(QObject* parent) : QObject(parent) {}

int UserBiz::CreateUser(const QString& name)
{
    static int nextId = 1000;
    int id = ++nextId;

    QJsonObject user;
    user["id"] = id;
    user["name"] = name;
    user["created_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    UserRepo::instance().Save(user);

    emit userCreated(user);        // 领域事件
    return id;
}

QJsonObject UserBiz::GetUser(int id)
{
    return UserRepo::instance().FindById(id);
}

void UserBiz::DeleteUser(int id)
{
    UserRepo::instance().Remove(id);
}
