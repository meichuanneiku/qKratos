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

bool DataBaseManager::connect()
{
    if (!m_data.db.open())
    {
        qWarning() << "Database connection failed:" << m_data.db.lastError().text();
        return false;
    }
    qDebug("Database connected successfully");

    if (!m_data.pgdb.open())
    {
        qWarning() << "PostgreSQL Database connection failed:" << m_data.pgdb.lastError().text();
        return true;
    }
    qDebug("PostgreSQL Database connected successfully");
    return true;
}

DataBaseManager::DataBaseManager()
{
    Config* pConf = Config::instance();
    if(pConf){
        //多个QSqlDatabase::addDatabase连接时必须添加各自的connectionName
        m_data.db = QSqlDatabase::addDatabase(pConf->data().dmsql.driverType, "dm_connection");
        m_data.db.setDatabaseName(pConf->data().dmsql.dbname);
        m_data.db.setPort(pConf->data().dmsql.port);
        m_data.db.setHostName(pConf->data().dmsql.host);
        m_data.db.setUserName(pConf->data().dmsql.user);
        m_data.db.setPassword(pConf->data().dmsql.password);

        //redis

        //pqsql
        m_data.pgdb = QSqlDatabase::addDatabase("QPSQL", "pgsql_connection");
        m_data.pgdb.setDatabaseName(pConf->data().pgsql.dbname);
        m_data.pgdb.setPort(pConf->data().pgsql.port);
        m_data.pgdb.setHostName(pConf->data().pgsql.host);
        m_data.pgdb.setUserName(pConf->data().pgsql.user);
        m_data.pgdb.setPassword(pConf->data().pgsql.password);

        //mysql
    }

}
