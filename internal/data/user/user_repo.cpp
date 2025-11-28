#include "user_repo.h"

#define UserTableName       "XXGY.YHGL_USER"
#define RoleTableName       "XXGY.YHGL_JSB"
#define FwkzTableName       "XXGY.YHGL_FWKZ"

#define UserTableName2       "XXGY.YHGL_USER_2"
#define RoleTableName2       "XXGY.YHGL_JSB_2"
#define FwkzTableName2       "XXGY.YHGL_FWKZ_2"

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
