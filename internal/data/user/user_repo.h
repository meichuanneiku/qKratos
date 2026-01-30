#ifndef USERREPO_H
#define USERREPO_H

#include <QHash>
#include <QJsonObject>
#include <QDebug>
#include "../data.h"
#include "../../pkg/permission/data_scope.h"

#include "../../pkg/global/global.h"

using namespace qKratos::Permission;
//#include <QSqlQuery>
class UserRepo
{
public:
    static UserRepo& instance()
    {
        static UserRepo repo;   // C++11 起支持的线程安全单例
        return repo;
    }
    //    static UserRepo& instance() { static UserRepo r; return r; }

    void Save(const QJsonObject& u) { storage[u["id"].toInt()] = u; }
    QSqlQuery FindById(const int& systemId, QString id) const;// { return storage.value(id); }
    bool Remove(int id);

    QueryResult FindByName(const int& systemId, const QString& name) const;// { return storage.value(id); }

    QueryResult ListUsers(const int& systemId, const QString& fxtid,const QString& jsid,
                          const QString& zxzt, const QString& yhmc,
                          const QString& dlcsCondition, const int& dlcs,
                          const int& page, const int& pageSize) const;


   QueryResult deviceParamPush(const int &SJJG, const int &index, const QJsonArray &xhdIdArray);

private:
    // 私有构造函数，防止外部实例化
    UserRepo() = default;

    QHash<int, QJsonObject> storage;
};

#endif // USERREPO_H
