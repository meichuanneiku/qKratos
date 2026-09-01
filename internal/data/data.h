#ifndef DATA_H
#define DATA_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QThreadStorage>

struct Data {
    QSqlDatabase db;
    QSqlDatabase pgdb;
    QSqlDatabase mysqldb;
};

class DataBaseManager
{
public:

    ~DataBaseManager();

    static DataBaseManager *instance();

    bool connect();

    const Data& data() const { return m_data; }
    QSqlDatabase db(int systemId = 1);

    const QString dbError() const { return m_data.db.lastError().text(); }
private:
    DataBaseManager();

    static DataBaseManager* m_instance;

    Data m_data;

    // 线程本地连接缓存，每个线程独立连接
    mutable QThreadStorage<QSqlDatabase> m_threadLocalDb;
    QSqlDatabase createConnection(int systemId) const;
};

#endif // DATA_H
