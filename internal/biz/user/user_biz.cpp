#include "user_biz.h"

#include <QDateTime>

#include "../../pkg/middleware/auth.h"

using namespace qKratos::Middleware;

int UserBiz::CreateUser(const int& systemId, const QString& name)
{
    static int nextId = 1000;
    int id = ++nextId;

    QJsonObject user;
    user["id"] = id;
    user["name"] = name;
    user["created_at"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    UserRepo::instance().Save(systemId, user);

    return id;
}

QJsonObject UserBiz::GetUser(const int& systemId, QString condition)
{
    QueryResult qResult = UserRepo::instance().FindById(systemId, condition);
    if (!qResult.isSuccess()) {
        return QJsonObject{};
    }
    QSqlQuery userInfo = qResult.query;
    QJsonObject user;
    user["YHID"] = userInfo.value(0).toString();
    user["YHMC"] = userInfo.value(3).toString();
    user["ZXZT"] = userInfo.value(4).toBool();
    return user;
}

QJsonObject UserBiz::FindById(const int& systemId, QString id)
{
    QJsonObject user;
    QueryResult qResult = UserRepo::instance().FindById(systemId, id);
    if (!qResult.isSuccess()) {
        return user;
    }
    QSqlQuery userInfo = qResult.query;

    user["YHID"] = userInfo.value(0).toString();
    user["YHMC"] = userInfo.value(3).toString();
    user["ZXZT"] = userInfo.value(4).toBool();

    return user;
}

QJsonObject UserBiz::ListUsers(const int& systemId, const QString& page)
{
    QueryResult qResult = UserRepo::instance().ListUsers(systemId, "", "", "", "", "", -1, page.toInt(), 20);
    if (!qResult.isSuccess()) {
        return QJsonObject{};
    }
    QSqlQuery result = qResult.query;
    QJsonArray users;
    while (result.next()) {
        QJsonObject u;
        u["YHID"] = result.value(0).toString();
        u["YHMC"] = result.value(3).toString();
        u["ZXZT"] = result.value(4).toBool();
        users.append(u);
    }
    QJsonObject data;
    data["users"] = users;
    data["total"] = users.size();
    return data;
}

bool UserBiz::DeleteUser(const int& systemId, int id)
{
    return UserRepo::instance().Remove(systemId, id);
}

bool UserBiz::UpdateUser(const int& systemId, const QJsonObject& user)
{
    return UserRepo::instance().Update(systemId, user);
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

JsonObjectResult UserBiz::deviceParamPush(const QJsonObject &doc)
{
    JsonObjectResult  jResult;
    int SJJG = doc["SJJG"].toInt();
    int index = doc["index"].toInt();
    QJsonArray xhdIdArray = doc["XHDIDLB"].toArray();

    QueryResult userListResult = UserRepo::instance().deviceParamPush(SJJG, index, xhdIdArray);

    if(!userListResult.isSuccess()){
        jResult.errorCode = userListResult.errorCode;
        return jResult;
    }

    QSqlQuery query = userListResult.query;


    // 假设已执行查询，QSqlQuery query 包含结果
    QJsonArray resultArray;

    // 用于按 XHDID 分组
    QMap<QString, QJsonObject> groupMap;

    while (query.next()) {
        QString xhdid = query.value("XHDID").toString();
        QString ms = query.value("MS").toString();
        QDateTime time = query.value("time").toDateTime();
        int value = query.value("value").toInt();

        // 如果是新 XHDID，初始化对象
        if (!groupMap.contains(xhdid)) {
            QJsonObject obj;
            obj["MS"] = ms;
            obj["XHDID"] = xhdid;
            obj["data"] = QJsonArray();
            groupMap[xhdid] = obj;
        }

        // 构建 data 点（time 用毫秒时间戳）
        QJsonObject dataPoint;
        dataPoint["time"] = QString::number(time.toMSecsSinceEpoch());
        dataPoint["value"] = value;

        // 添加到对应 XHDID 的 data 数组
        QJsonArray dataArray = groupMap[xhdid]["data"].toArray();
        dataArray.append(dataPoint);
        groupMap[xhdid]["data"] = dataArray;
    }

    // 将分组结果转为数组
    for (auto it = groupMap.constBegin(); it != groupMap.constEnd(); ++it) {
        resultArray.append(it.value());
    }

    QJsonObject result;
    result["list"] = resultArray;
    result["total"] = resultArray.count();


    jResult.data = result;
    return jResult;


}
