#include "user_repo.h"
#include "../../conf/conf.h"

static QString userTable(int systemId) {
    return systemId == 1 ? Config::instance()->tables().user : Config::instance()->tables().user2;
}

static QString roleTable(int systemId) {
    return systemId == 1 ? Config::instance()->tables().role : Config::instance()->tables().role2;
}

QueryResult UserRepo::FindById(const int &systemId, QString id) const
{
    QueryResult qResult;
    auto& tbl = Config::instance()->tables();
    QString sql = QString("select a.*,b.FXTMC,c.JSMC,c.QX from %1 a left join %2 b on a.FXTID=b.FXTID left join %3 c on a.JSID=c.JSID")
            .arg(userTable(systemId)).arg(tbl.fxtb).arg(roleTable(systemId));

    if(id > 0)
        sql += " where YHID = ?";
    sql += " ORDER BY a.ZXZT DESC";

    QSqlQuery query(DataBaseManager::instance()->db(systemId));
    query.prepare(sql);
    query.addBindValue(id);

    if (!query.exec() || !query.next()) {
        qWarning() << QString("[%1]Select query failed:%2").arg(sql, query.lastError().text());
        qResult.errorCode = UnknownError;
        qResult.errorMessage = query.lastError().text();
        return qResult;
    }

    qResult.query = query;
    return qResult;
}

void UserRepo::Save(const int& systemId, const QJsonObject& u)
{
    QSqlQuery query(DataBaseManager::instance()->db(systemId));
    query.prepare(QString("INSERT INTO %1 (YHMC, PASSWORD, ZXZT) VALUES (?, ?, ?)").arg(userTable(systemId)));
    query.addBindValue(u["name"].toString());
    query.addBindValue(u["password"].toString());
    query.addBindValue(u["zxzt"].toBool(false));
    if (!query.exec()) {
        qWarning() << QString("Save user failed:%1").arg(query.lastError().text());
    }
}

bool UserRepo::Update(const int& systemId, const QJsonObject& u)
{
    QSqlQuery query(DataBaseManager::instance()->db(systemId));
    query.prepare(QString("UPDATE %1 SET YHMC = ?, ZXZT = ? WHERE YHID = ?").arg(userTable(systemId)));
    query.addBindValue(u["YHMC"].toString());
    query.addBindValue(u["ZXZT"].toBool());
    query.addBindValue(u["YHID"].toInt());
    if (!query.exec()) {
        qWarning() << QString("Update user failed:%1").arg(query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool UserRepo::Remove(const int& systemId, int id)
{
    QSqlQuery query(DataBaseManager::instance()->db(systemId));
    query.prepare(QString("DELETE FROM %1 WHERE YHID = ?").arg(userTable(systemId)));
    query.addBindValue(id);
    if (!query.exec()) {
        qWarning() << QString("Remove user failed:%1").arg(query.lastError().text());
        return false;
    }
    return query.numRowsAffected() > 0;
}

QueryResult UserRepo::FindByName(const int &systemId, const QString &name) const
{
    QueryResult qResult;
    QString sql = QString("select a.YHID,a.PASSWORD, c.JSMC, c.QX "
                          "from %1 a left join %2 c on a.JSID=c.JSID")
            .arg(userTable(systemId)).arg(roleTable(systemId));

    sql += " where YHMC = ?";

    QSqlQuery query(DataBaseManager::instance()->db(systemId));
    query.prepare(sql);
    query.addBindValue(name);

    if (!query.exec() || !query.next()) {
        qWarning() << QString("[%1]Select query failed:%2").arg(sql, query.lastError().text());
        qResult.errorCode = UnknownError;
        qResult.errorMessage = query.lastError().text();
        return qResult;
    }

    qResult.query = query;
    return qResult;
}

QueryResult UserRepo::FindByNameAndPassword(const int &systemId, const QString &name, const QString &password) const
{
    QueryResult qResult;
    QString sql = QString("select a.YHID, c.JSMC, c.QX "
                          "from %1 a left join %2 c on a.JSID=c.JSID")
            .arg(userTable(systemId)).arg(roleTable(systemId));

    sql += " where YHMC = ? AND PASSWORD = ?";

    QSqlQuery query(DataBaseManager::instance()->db(systemId));
    query.prepare(sql);
    query.addBindValue(name);
    query.addBindValue(password);

    if (!query.exec() || !query.next()) {
        qWarning() << QString("[%1]Select query failed:%2").arg(sql, query.lastError().text());
        qResult.errorCode = UnknownError;
        qResult.errorMessage = query.lastError().text();
        return qResult;
    }

    qResult.query = query;
    return qResult;
}

QueryResult UserRepo::ListUsers(const int &systemId, const QString &fxtid, const QString &jsid,
                                const QString &zxzt, const QString &yhmc,
                                const QString &dlcsCondition, const int &dlcs,
                                const int &page, const int &pageSize) const
{
    QueryResult qResult;
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

    QString whereClause = whereConditions.isEmpty() ? "" : " WHERE " + whereConditions.join(" AND ");

    auto& tbl = Config::instance()->tables();
    QString sql = QString(
                "SELECT a.DHFJ, a.DLCS, a.DLIP, a.DLSJ, a.FXTID, a.JSID, a.YHMC, a.ZXZT, "
                "       b.FXTMC, c.JSMC, c.QX, "
                "       COUNT(*) OVER() AS total_records "
                "FROM %1 a "
                "LEFT JOIN %2 b ON a.FXTID = b.FXTID "
                "LEFT JOIN %3 c ON a.JSID = c.JSID "
                "%4 "
                "ORDER BY a.ZXZT DESC"
                ).arg(userTable(systemId)).arg(tbl.fxtb).arg(roleTable(systemId)).arg(whereClause);

    if (page > 0 && pageSize > 0) {
        sql += " LIMIT ? OFFSET ?";
        bindValues << pageSize << ((page - 1) * pageSize);
    }

    QSqlQuery query(DataBaseManager::instance()->db(systemId));
    query.prepare(sql);
    for (const auto &v : bindValues) {
        query.addBindValue(v);
    }

    if (!query.exec()) {
        qWarning() << "Database query failed:" << query.lastError().text()
                   << "SQL:" << query.lastQuery();
        qResult.errorCode = UnknownError;
        return qResult;
    }

    qResult.query = query;
    return qResult;
}

QueryResult UserRepo::deviceParamPush(const int &SJJG, const int &index, const QJsonArray &xhdIdArray)
{
    QueryResult qResult;
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QStringList placeholders;
    for (int i = 0; i < xhdIdArray.size(); ++i) {
        placeholders << "?";
    }
    QString inClause = placeholders.join(",");

    QString sql = QString(R"(
        SELECT XHDID, MS, CJSJ AS time, DQZ AS value
        FROM XXGY.SJGL_LLCX
        WHERE XHDID IN (%1)
          AND CJSJ >= DATEADD(MINUTE, ?, ?)
          AND CJSJ <= ?
        ORDER BY XHDID, CJSJ ASC
    )").arg(inClause);

    QSqlQuery query(DataBaseManager::instance()->db(1));
    query.prepare(sql);

    for (const auto &val : xhdIdArray) {
        query.addBindValue(val.toString());
    }

    query.addBindValue(index ? 0 : -SJJG);
    query.addBindValue(currentTime);
    query.addBindValue(currentTime);

    if (!query.exec()) {
        qWarning() << "Database query failed:" << query.lastError().text()
                   << "SQL:" << query.lastQuery();
        qResult.errorCode = UnknownError;
        return qResult;
    }

    qResult.query = query;
    return qResult;
}
