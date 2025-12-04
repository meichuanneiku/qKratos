#ifndef DATA_SCOPE_H
#define DATA_SCOPE_H

#include <QSqlQuery>
#include <QString>
//#include "../../pkg/jwt/jwt.h"
//#include "../middleware/auth_middleware.h"  // currentUser()

//using namespace qKratos::JWT;

#include "../middleware/auth.h"
using namespace qKratos::Middleware;
namespace qKratos::Permission {

// 数据权限枚举（和 Go 版完全一致）
enum DataScope : int {
    All         = 1,  // 全部
    RoleDept    = 2,  // 本角色部门
    Dept        = 3,  // 本部门
    DeptAndSub  = 4,  // 本部门及子部门
    Self        = 5   // 仅本人
};

// 核心函数：给任意查询追加数据权限，返回可继续链式调用的 QSqlQuery
inline QSqlQuery applyDataScope(QSqlQuery query, const QString& tableName, const QString& createByField = "create_by")
{
    QVariant userVar = currentUser();
    if (!userVar.isValid()) {
        query.prepare(QString("SELECT * FROM %1 WHERE 1=0").arg(tableName));
        return query;
    }

   Claims claims = userVar.value<Claims>();

    QString whereClause;
    QVector<QVariant> binds;

    switch (claims.dataScope) {
    case DataScope::All:
        whereClause = "1=1";
        break;

    case DataScope::RoleDept: {
        // 本角色部门下的所有用户
        whereClause = QString("%1 IN ("
                              "SELECT su.id FROM sys_users su "
                              "JOIN sys_role_depts srd ON su.dept_id = srd.dept_id "
                              "WHERE srd.role_id = ?"
                              ")").arg(createByField);
        binds << claims.roleId;
        break;
    }

    case DataScope::Dept: {
        whereClause = QString("%1 IN (SELECT id FROM sys_users WHERE dept_id = ?)").arg(createByField);
        binds << claims.departmentId;
        break;
    }

    case DataScope::DeptAndSub: {
        QString pattern = "%/" + QString::number(claims.departmentId) + "/%";
        whereClause = QString("%1 IN ("
                              "SELECT id FROM sys_users "
                              "WHERE dept_id IN ("
                              "SELECT id FROM sys_depts WHERE dept_path LIKE ?"
                              ")"
                              ")").arg(createByField);
        binds << pattern;
        break;
    }

    case DataScope::Self:
    default:
        whereClause = QString("%1 = ?").arg(createByField);
        binds << claims.sub.toLongLong();
        break;
    }

    // 追加 WHERE 条件（支持已有条件）
    QString sql = query.lastQuery();
    if (sql.contains("WHERE", Qt::CaseInsensitive)) {
        sql += " AND " + whereClause;
    } else {
        sql += " WHERE " + whereClause;
    }

    query.prepare(sql);
    // 重新绑定原始参数 + 数据权限参数
    for (int i = 0; i < query.boundValues().size(); ++i) {
        query.addBindValue(query.boundValue(i));
    }
    for (const auto& v : binds) {
        query.addBindValue(v);
    }

    return query;
}

} // namespace qratos::permission

#endif // DATA_SCOPE_H
