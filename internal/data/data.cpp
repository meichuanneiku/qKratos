#include "data.h"
#include "../conf/conf.h"

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

QSqlDatabase DataBaseManager::db(int systemId) const
{
    switch (systemId) {
    case 1:  return m_data.db;
    case 2:  return m_data.pgdb;
    default: return m_data.mysqldb;
    }
}
