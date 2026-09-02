#ifndef DATA_H
#define DATA_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QThreadStorage>
#include <QAtomicInt>

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

    // 连接数监控
    int activeConnections() const { return m_activeConnections.load(); }
    int maxConnections() const;

    const QString dbError() const { return m_data.db.lastError().text(); }
private:
    DataBaseManager();

    static DataBaseManager* m_instance;

    Data m_data;

    // 线程本地连接缓存，每个线程独立连接
    mutable QThreadStorage<QSqlDatabase> m_threadLocalDb;
    QSqlDatabase createConnection(int systemId) const;

    // 连接计数器
    QAtomicInt m_activeConnections;
};

#endif // DATA_H
