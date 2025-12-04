#include "user_biz.h"

#include <QDateTime>

#include "../../pkg/middleware/auth.h"

using namespace qKratos::Middleware;

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

bool UserBiz::DeleteUser(int id)
{
    return UserRepo::instance().Remove(id);
}

QJsonObject UserBiz::UserLogin(const int& systemId, const QString &name, const QString &password)
{
    //对密码进行加密
    QString passwordHash = QCryptographicHash::hash(password.toLatin1(), QCryptographicHash::Sha1).toHex();

    QSqlQuery loginInfo = UserRepo::instance().FindByNameAndPassword(systemId, name, passwordHash);


    if(!loginInfo.size()){
        return QJsonObject{};
    }

    // 3. 签发 JWT
    Claims claims;
    claims.sub = loginInfo.value(0).toString();                    // 用户ID
    claims.name = name;
    claims.roles = QStringList() <<loginInfo.value(1).toString();

    //    QString token = sign(claims);
    QString token = JwtHelper::sign(claims);

    // 4. 返回 token + 用户信息
    QJsonObject data{
        {"token", token},
        {"user_id", claims.sub},
        {"name",    claims.name},
        {"roles",   QJsonArray::fromStringList(claims.roles)},
        {"qx",   loginInfo.value(2).toString()}
    };
    return data;
}
