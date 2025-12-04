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

bool UserRepo::Remove(int id)
{
    return storage.remove(id);
}

QSqlQuery UserRepo::FindByName(const int &systemId, const QString &name) const
{

}

QSqlQuery UserRepo::FindByNameAndPassword(const int &systemId, const QString &name, const QString &password) const
{
    QString sql = QString("select a.YHID, c.JSMC, c.QX "
                          "from %1 a left join %2 c on a.JSID=c.JSID")
            .arg(systemId == 1 ? UserTableName : UserTableName2).arg(systemId == 1 ? RoleTableName : RoleTableName2);

    sql += " where YHMC = ? AND PASSWORD = ?";


    QJsonObject result;
    QSqlQuery query( DataBaseManager::instance()->data().db);
    query.prepare(sql);
    query.addBindValue(name);
    query.addBindValue(password);

    if (!query.exec() || !query.next()) {
        qWarning() << QString("[%1]Select query failed:%2").arg(sql, query.lastError().text());

        qDebug()<< "query.lastError().isValid()= " <<query.lastError().isValid();
        qDebug()<< "query.lastError().text()= " <<query.lastError().text();
        qDebug()<< "query.isValid()= " <<query.isValid();
        qDebug()<< "query.isActive()= " <<query.isActive();
        qDebug()<< "query.isSelect()= " <<query.isSelect();
        qDebug()<< "query.size()= " <<query.size();

        return query;
    }

    return query;
}

QSqlQuery UserRepo::ListUsers(const int &systemId, QString page) const
{
      QSqlQuery query( DataBaseManager::instance()->data().db);

      // 基础查询 + 数据权限
          query = applyDataScope(query, systemId == 1 ? UserTableName : UserTableName2);

          // 继续追加业务条件（比如搜索、状态、分页等）
//          QString keyword = req.query().queryItemValue("keyword");
//          if (!keyword.isEmpty()) {
//              query.exec(QString("AND (username LIKE '%%1%' OR name LIKE '%%1%')").arg(keyword));
//          }

          query.exec("AND status = ?");
          query.addBindValue(1); // 状态为启用

          query.exec("ORDER BY create_time DESC LIMIT 20 OFFSET 0");

          if (!query.exec() || !query.next()) {
              return query;
          }
          return query;
}
