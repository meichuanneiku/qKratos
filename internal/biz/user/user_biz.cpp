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

QJsonObject UserBiz::GetUser(const int& systemId, QString condition)
{

}

QJsonObject UserBiz::FindById(const int& systemId, QString id)
{
    QJsonObject user;
    QSqlQuery userInfo = UserRepo::instance().FindById(systemId, id);
////    QJsonObject user;
////
    if (userInfo.isNull("YHID")){
        return user;
    }
//    qDebug()<<"userInfo.isValid = " << userInfo.isValid();
//    qDebug()<<"userInfo.isNull = " <<userInfo.isNull("YHID");

    user["YHID"] = userInfo.value(0).toString();
    user["YHMC"] = userInfo.value(3).toString();
    user["ZXZT"] = userInfo.value(4).toBool();

    return user;
//    return QJsonObject{
//        {
//            {"YHID", userInfo.value(0).toString()},
//            {"YHMC", userInfo.value(3).toString()},
//            {"ZXZT", userInfo.value(4).toBool()}
//        }
//    };
}

int UserBiz::DeleteUser(int id)
{
    return UserRepo::instance().Remove(id);
}
