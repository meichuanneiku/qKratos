#include "data.h"
#include "../conf/conf.h"
#include <QThread>

DataBaseManager* DataBaseManager::m_instance = nullptr;

DataBaseManager *DataBaseManager::instance()
{
    if(!m_instance)
    {
        m_instance = new DataBaseManager;
    }
    return m_instance;
}

DataBaseManager::~DataBaseManager()
{
    if (m_data.db.isOpen()) m_data.db.close();
    if (m_data.pgdb.isOpen()) m_data.pgdb.close();
    if (m_data.mysqldb.isOpen()) m_data.mysqldb.close();
}

DataBaseManager::DataBaseManager()
{
    Config* pConf = Config::instance();
    if (!pConf) return;

    // 达梦数据库 (systemId == 1)
    m_data.db = QSqlDatabase::addDatabase(pConf->data().dmsql.driverType, "dm_connection");
    m_data.db.setDatabaseName(pConf->data().dmsql.dbname);
    m_data.db.setPort(pConf->data().dmsql.port);
    m_data.db.setHostName(pConf->data().dmsql.host);
    m_data.db.setUserName(pConf->data().dmsql.user);
    m_data.db.setPassword(pConf->data().dmsql.password);

    // PostgreSQL (systemId == 2)
    m_data.pgdb = QSqlDatabase::addDatabase("QPSQL", "pgsql_connection");
    m_data.pgdb.setDatabaseName(pConf->data().pgsql.dbname);
    m_data.pgdb.setPort(pConf->data().pgsql.port);
    m_data.pgdb.setHostName(pConf->data().pgsql.host);
    m_data.pgdb.setUserName(pConf->data().pgsql.user);
    m_data.pgdb.setPassword(pConf->data().pgsql.password);

    // MySQL
    m_data.mysqldb = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection");
    m_data.mysqldb.setDatabaseName(pConf->data().mysql.dbname);
    m_data.mysqldb.setPort(pConf->data().mysql.port);
    m_data.mysqldb.setHostName(pConf->data().mysql.host);
    m_data.mysqldb.setUserName(pConf->data().mysql.user);
    m_data.mysqldb.setPassword(pConf->data().mysql.password);
}

bool DataBaseManager::connect()
{
    bool okDM = m_data.db.open();
    if (okDM) qInfo("DM8 connected successfully");
    else qWarning() << "DM8 connection failed:" << m_data.db.lastError().text();

    bool okPG = m_data.pgdb.open();
    if (okPG) qInfo("PostgreSQL connected successfully");
    else qWarning() << "PostgreSQL connection failed:" << m_data.pgdb.lastError().text();

    bool okMySQL = m_data.mysqldb.open();
    if (okMySQL) qInfo("MySQL connected successfully");
    else qWarning() << "MySQL connection failed:" << m_data.mysqldb.lastError().text();

    return okDM || okPG || okMySQL;
}

QSqlDatabase DataBaseManager::createConnection(int systemId) const
{
    Config* pConf = Config::instance();
    if (!pConf) return QSqlDatabase();

    // 为每个线程生成唯一的连接名称
    QString connName = QString("thread_%1_%2").arg(reinterpret_cast<quintptr>(QThread::currentThread())).arg(systemId);

    switch (systemId) {
    case 1: {
        QSqlDatabase conn = QSqlDatabase::addDatabase(pConf->data().dmsql.driverType, connName);
        conn.setDatabaseName(pConf->data().dmsql.dbname);
        conn.setPort(pConf->data().dmsql.port);
        conn.setHostName(pConf->data().dmsql.host);
        conn.setUserName(pConf->data().dmsql.user);
        conn.setPassword(pConf->data().dmsql.password);
        if (conn.open()) {
            qInfo() << "Thread-local DM8 connection created for thread" << QThread::currentThread();
        } else {
            qWarning() << "Thread-local DM8 connection failed:" << conn.lastError().text();
        }
        return conn;
    }
    case 2: {
        QSqlDatabase conn = QSqlDatabase::addDatabase("QPSQL", connName);
        conn.setDatabaseName(pConf->data().pgsql.dbname);
        conn.setPort(pConf->data().pgsql.port);
        conn.setHostName(pConf->data().pgsql.host);
        conn.setUserName(pConf->data().pgsql.user);
        conn.setPassword(pConf->data().pgsql.password);
        if (conn.open()) {
            qInfo() << "Thread-local PostgreSQL connection created for thread" << QThread::currentThread();
        } else {
            qWarning() << "Thread-local PostgreSQL connection failed:" << conn.lastError().text();
        }
        return conn;
    }
    default: {
        QSqlDatabase conn = QSqlDatabase::addDatabase("QMYSQL", connName);
        conn.setDatabaseName(pConf->data().mysql.dbname);
        conn.setPort(pConf->data().mysql.port);
        conn.setHostName(pConf->data().mysql.host);
        conn.setUserName(pConf->data().mysql.user);
        conn.setPassword(pConf->data().mysql.password);
        if (conn.open()) {
            qInfo() << "Thread-local MySQL connection created for thread" << QThread::currentThread();
        } else {
            qWarning() << "Thread-local MySQL connection failed:" << conn.lastError().text();
        }
        return conn;
    }
    }
}

QSqlDatabase DataBaseManager::db(int systemId)
{
    // 主线程使用原始连接（向后兼容）
    // Qt 5.12 QSqlDatabase 没有 thread() 方法，通过 connectionName 判断
    QString currentConnName = QSqlDatabase::connectionNames().isEmpty()
        ? QString()
        : QSqlDatabase::connectionNames().first();

    // 非主线程使用线程本地连接
    if (m_threadLocalDb.hasLocalData()) {
        QSqlDatabase& cached = m_threadLocalDb.localData();
        if (cached.isOpen()) return cached;
    }

    // 主线程且已有连接，直接返回原始连接
    if (!m_threadLocalDb.hasLocalData()) {
        switch (systemId) {
        case 1:  if (m_data.db.isOpen()) return m_data.db; break;
        case 2:  if (m_data.pgdb.isOpen()) return m_data.pgdb; break;
        default: if (m_data.mysqldb.isOpen()) return m_data.mysqldb; break;
        }
    }

    // 首次调用，创建线程本地连接
    QSqlDatabase conn = createConnection(systemId);
    if (conn.isOpen()) {
        m_threadLocalDb.setLocalData(conn);
    }
    return conn;
}
