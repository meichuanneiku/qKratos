#ifndef USERREPO_H
#define USERREPO_H

#include <QHash>
#include <QJsonObject>
#include <QDebug>
#include "../data.h"

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
    int Remove(int id) { return storage.remove(id); }

private:
    // 私有构造函数，防止外部实例化
    UserRepo() = default;

    QHash<int, QJsonObject> storage;
};

#endif // USERREPO_H
