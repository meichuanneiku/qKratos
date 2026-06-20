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

    void Save(const int& systemId, const QJsonObject& u);
    bool Update(const int& systemId, const QJsonObject& u);
    QueryResult FindById(const int& systemId, QString id) const;
    bool Remove(const int& systemId, int id);

    QueryResult FindByName(const int& systemId, const QString& name) const;

    QueryResult FindByNameAndPassword(const int& systemId, const QString& name, const QString &password) const;

    QueryResult ListUsers(const int& systemId, const QString& fxtid,const QString& jsid,
                          const QString& zxzt, const QString& yhmc,
                          const QString& dlcsCondition, const int& dlcs,
                          const int& page, const int& pageSize) const;


   QueryResult deviceParamPush(const int &SJJG, const int &index, const QJsonArray &xhdIdArray);

private:
    UserRepo() = default;
};

#endif // USERREPO_H
