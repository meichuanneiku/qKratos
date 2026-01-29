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

JsonObjectResult UserBiz::UserLogin(const int& systemId, const QString &name, const QString &password)
{
    JsonObjectResult  result;
    //对密码进行加密
    QString passwordHash = QCryptographicHash::hash(password.toLatin1(), QCryptographicHash::Sha1).toHex();

    QueryResult loginResult = UserRepo::instance().FindByName(systemId, name);

    if(!loginResult.isSuccess()){
        result.errorCode = loginResult.errorCode;
        return result;
    }

    QSqlQuery loginInfo = loginResult.query;
    QString storedHash = loginInfo.value("PASSWORD").toString();
    if(storedHash != passwordHash){
        qCritical()<<"用户密码错误";
        result.errorCode = UserNotFound;
        return result;
    }


    // 3. 签发 JWT
    Claims claims;
    claims.sub = loginInfo.value(0).toString();                    // 用户ID
    claims.name = name;
    claims.roles = QStringList() <<loginInfo.value(1).toString();

    claims.roleId = 0;
    claims.departmentId = 0;
    claims.dataScope = 1;

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

    result.data = data;
    return result;
}

JsonObjectResult UserBiz::UserList(const int &systemId, const QString &fxtid, const QString &jsid, const QString &zxzt, const QString &yhmc, const QString &dlcsCondition, const int &dlcs, const int &page, const int &pageSize)
{
    JsonObjectResult  jResult;

    QueryResult userListResult = UserRepo::instance().ListUsers(systemId, fxtid, jsid, zxzt, yhmc, dlcsCondition, dlcs, page, pageSize);
    if(!userListResult.isSuccess()){
        jResult.errorCode = userListResult.errorCode;
        return jResult;
    }

    QSqlQuery userListInfo = userListResult.query;
    int totalRecords = -1;
    int totalPage = 0;
    QJsonArray userArray;
    //解析用户列表数据
    while (userListInfo.next()) {
        if (totalRecords == -1) {
            totalRecords = userListInfo.value("total_records").toLongLong(); // 第一行就读总数
            totalPage = qCeil(totalRecords *1.0 / pageSize);
        }
        QJsonObject row;

        // 按字段名逐个提取（建议显式指定，避免依赖列顺序）
        row["DHFJ"] = userListInfo.value("DHFJ").toString();
        row["DLCS"] = userListInfo.value("DLCS").toInt();
        row["DLIP"] = userListInfo.value("DLIP").toString();
        row["DLSJ"] = userListInfo.value("DLSJ").toDateTime().toString(Qt::ISODate); // 或按需格式化
        row["FXTID"] = userListInfo.value("FXTID").toString();
        row["JSID"] = userListInfo.value("JSID").toString();
        row["YHMC"] = userListInfo.value("YHMC").toString();
        row["ZXZT"] = userListInfo.value("ZXZT").toString();
        row["FXTMC"] = userListInfo.value("FXTMC").toString();
        row["JSMC"] = userListInfo.value("JSMC").toString();
        row["QX"] = userListInfo.value("QX").toString();

        userArray.append(row);
    }

    if(userArray.isEmpty()){
        qWarning() << "用户列表为空 " << "SQL:" << userListInfo.lastQuery(); // 这里只有带占位符的 SQL;
    }

    QJsonObject data{
        {"list",    userArray},
        {"totalPage",   totalPage},
        {"totalRecords",   qMax(totalRecords, 0)}
    };
    jResult.data = data;
    return jResult;
}
