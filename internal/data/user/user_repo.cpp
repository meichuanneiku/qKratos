#include "user_repo.h"

#define UserTableName       "XXGY.YHGL_USER"
#define RoleTableName       "XXGY.YHGL_JSB"
#define FwkzTableName       "XXGY.YHGL_FWKZ"

#define UserTableName2       "XXGY.YHGL_USER_2"
#define RoleTableName2       "XXGY.YHGL_JSB_2"
#define FwkzTableName2       "XXGY.YHGL_FWKZ_2"

#define GET_USER_TABLE(systemId)   ((systemId) == 1 ? UserTableName : UserTableName2)
#define GET_ROLE_TABLE(systemId)   ((systemId) == 1 ? RoleTableName : RoleTableName2)
#define GET_FWKZ_TABLE(systemId)   ((systemId) == 1 ? FwkzTableName : FwkzTableName2)

QSqlQuery UserRepo::FindById(const int &systemId, QString id) const
{
    QString sql = QString("select a.*,b.FXTMC,c.JSMC,c.QX from %1 a left join %2 b on a.FXTID=b.FXTID left join %3 c on a.JSID=c.JSID")
            .arg(systemId == 1 ? UserTableName : UserTableName2).arg("XXGY.FXTB").arg(systemId == 1 ? RoleTableName : RoleTableName2);

    if(id > 0)
        sql += " where YHID = ?";
    // 3. 排序处理
    sql += " ORDER BY a.ZXZT DESC";

    QJsonObject result;
    QSqlQuery query( DataBaseManager::instance()->data().db);
    query.prepare(sql);
    query.addBindValue(id);

    if (!query.exec() || !query.next()) {
        qWarning() << QString("[%1]Select query failed:%2").arg(sql, query.lastError().text());
        return query;
    }

    return query;
}

bool UserRepo::Remove(int id)
{
    return storage.remove(id);
}

QueryResult UserRepo::FindByName(const int &systemId, const QString &name) const
{
    QueryResult qResult;

    QString sql = QString("select a.YHID,a.PASSWORD, c.JSMC, c.QX "
                          "from %1 a left join %2 c on a.JSID=c.JSID")
            .arg(systemId == 1 ? UserTableName : UserTableName2).arg(systemId == 1 ? RoleTableName : RoleTableName2);

    sql += " where YHMC = ?";


    QJsonObject result;
    QSqlQuery query( DataBaseManager::instance()->data().db);
    query.prepare(sql);
    query.addBindValue(name);


    // 3. 执行查询
    if (!query.exec()) {
        // 记录真实错误（仅内部日志，不暴露给用户）
        qWarning() << "Database query failed:" << query.lastError().text()
                   << "SQL:" << query.lastQuery(); // 这里只有带占位符的 SQL
        qResult.errorCode = UnknownError;
        return qResult;
    }

    // 4. 检查结果
    if (!query.next()) {
        // 用户不存在
        qWarning() << "用户不存在";
        qResult.errorCode = UserNotFound;
        return qResult;
    }

    qResult.query = query;
    return qResult;
}

QueryResult UserRepo::ListUsers(const int &systemId, const QString &fxtid, const QString &jsid, const QString &zxzt, const QString &yhmc, const QString &dlcsCondition, const int &dlcs, const int &page, const int &pageSize) const
{
    QueryResult qResult;

    QSqlQuery query( DataBaseManager::instance()->data().db);
    /*
    // 基础查询 + 数据权限
//    query = applyDataScope(query, systemId == 1 ? UserTableName : UserTableName2);

    QString sql = QString("select a.DHFJ, a.DLCS, a.DLIP, a.DLSJ, a.FXTID, a.JSID, a.YHMC, a.ZXZT, "
                          "b.FXTMC, c.JSMC, c.QX from %1 a left join %2 b on a.FXTID = b.FXTID left join %3 c on a.JSID = c.JSID")
            .arg(systemId == 1 ? UserTableName : UserTableName2).arg("XXGY.FXTB").arg(systemId == 1 ? RoleTableName : RoleTableName2);

    query.prepare(sql);

    // 继续追加业务条件（比如搜索、状态、分页等）
    if(!fxtid.isEmpty()){
        query.prepare("where a.FXTID = ?");
        query.addBindValue(fxtid);
    }
    if(!jsid.isEmpty()){
        query.prepare(" and a.JSID = ?");
        query.addBindValue(jsid);
    }
    if(!zxzt.isEmpty()){
        query.prepare(" and a.ZXZT = ?");
        query.addBindValue(zxzt);
    }
    if(!yhmc.isEmpty()){
        query.prepare(" and a.YHMC = ?");
        query.addBindValue(yhmc);
    }

    if (!dlcsCondition.isEmpty() && dlcs > -1){
        if(dlcsCondition != "all"){
            query.prepare(" and a.DLCS ? ?");
            query.addBindValue(dlcsCondition == "gte" ? ">=" : (dlcsCondition == "lte" ? "<=" : ""));
            query.addBindValue(dlcs);
        }
    }

    //排序
    query.prepare(" ORDER BY a.ZXZT DESC");

    //分页
    if(page > 0 && pageSize>0){
        query.prepare(" limit ?,?");
        query.addBindValue((page - 1) * pageSize);
        query.addBindValue(pageSize);
    }


//    query.exec("AND status = ?");
//    query.addBindValue(1); // 状态为启用

//    query.exec("ORDER BY create_time DESC LIMIT 20 OFFSET 0");
*/

    // 1. 确定表名
    QString userTable = GET_USER_TABLE(systemId);
    QString roleTable = GET_ROLE_TABLE(systemId);


    // 构建 WHERE 条件
    QStringList whereConditions;
    QList<QVariant> bindValues;

    auto addCond = [&](const QString &cond, const QVariant &val) {
        whereConditions << cond;
        bindValues << val;
    };

    if (!fxtid.isEmpty()) addCond("a.FXTID = ?", fxtid);
    if (!jsid.isEmpty()) addCond("a.JSID = ?", jsid);
    if (!zxzt.isEmpty()) addCond("a.ZXZT = ?", zxzt);
    if (!yhmc.isEmpty()) addCond("a.YHMC LIKE ?", QString("%%1%").arg(yhmc));
    if (!dlcsCondition.isEmpty() && dlcs > -1) {
        if (dlcsCondition == "gte") addCond("a.DLCS >= ?", dlcs);
        else if (dlcsCondition == "lte") addCond("a.DLCS <= ?", dlcs);
    }

    QString whereClause = whereConditions.isEmpty() ? "" : (" WHERE " + whereConditions.join(" AND "));

    // 主查询：使用窗口函数 COUNT(*) OVER()
    QString sql = QString(
                "SELECT a.DHFJ, a.DLCS, a.DLIP, a.DLSJ, a.FXTID, a.JSID, a.YHMC, a.ZXZT, "
                "       b.FXTMC, c.JSMC, c.QX, "
                "       COUNT(*) OVER() AS total_records "
                "FROM %1 a "
                "LEFT JOIN XXGY.FXTB b ON a.FXTID = b.FXTID "
                "LEFT JOIN %2 c ON a.JSID = c.JSID "
                "%3 "
                "ORDER BY a.ZXZT DESC"
                ).arg(userTable).arg(roleTable).arg(whereClause);

    // 分页
    if (page > 0 && pageSize > 0) {
        sql += " LIMIT ? OFFSET ?";
        bindValues << pageSize << ((page - 1) * pageSize);
    }

    query.prepare(sql);
    for (const auto &v : bindValues) {
        query.addBindValue(v);
    }

    if (!query.exec()) {
        // 记录真实错误（仅内部日志，不暴露给用户）
        qWarning() << "Database query failed:" << query.lastError().text()
                   << "SQL:" << query.lastQuery(); // 这里只有带占位符的 SQL
        qResult.errorCode = UnknownError;
        return qResult;
    }

    //    // 4. 检查结果
    //    if (!query.next()) {
    //        qWarning() << "用户列表为空 " << "SQL:" << query.lastQuery(); // 这里只有带占位符的 SQL;
    //    }

    qResult.query = query;
    return qResult;
}

QueryResult UserRepo::deviceParamPush(const int &SJJG, const int &index, const QJsonArray &xhdIdArray)
{
    QueryResult qResult;

//    int SJJG = doc["SJJG"].toInt();
//    int index = doc["index"].toInt();
    QString currentTime =  QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

//    QJsonArray xhdIdArray = doc["XHDIDLB"].toArray();

    // 1. 生成独立占位符（关键！）
    QStringList placeholders;
    for (int i = 0; i < xhdIdArray.size(); ++i) {
        placeholders << "?";
    }
    QString inClause = placeholders.join(",");

    // 2. 构建 SQL（DATEADD 单位加单引号 'MINUTE'）
    QString sql = QString(R"(
        SELECT XHDID, MS, CJSJ AS time, DQZ AS value
        FROM XXGY.SJGL_LLCX
        WHERE XHDID IN (%1)
          AND CJSJ >= DATEADD(MINUTE, ?, ?)
          AND CJSJ <= ?
        ORDER BY XHDID, CJSJ ASC
    )").arg(inClause);


    QSqlQuery query(DataBaseManager::instance()->data().db);

    query.prepare(sql);

    // 3. 绑定 ID（不加单引号！）
    for (const auto &val : xhdIdArray) {
        query.addBindValue(val.toString());  // ✅ Qt 自动加引号
    }

    // 4. 绑定时间参数（不加单引号！24小时制）
    query.addBindValue(index ? 0 : -SJJG);        // 负数表示往前推
    query.addBindValue(currentTime);  // 起始时间
    query.addBindValue(currentTime);  // 结束时间

    if (!query.exec()) {
        // 记录真实错误（仅内部日志，不暴露给用户）
        qWarning() << "Database query failed:" << query.lastError().text()
                   << "SQL:" << query.lastQuery(); // 这里只有带占位符的 SQL
        QMapIterator<QString, QVariant> i(query.boundValues());
        while (i.hasNext()) {
            i.next();
            qWarning() << i.key().toUtf8().data() << ": "
                       << i.value().toString().toUtf8().data();
        }
        qResult.errorCode = UnknownError;
        return qResult;
    }

    qResult.query = query;
    return qResult;
}
