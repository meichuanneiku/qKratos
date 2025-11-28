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
    if (m_data.db.open())
    {
        qDebug("Database connected successfully");
        return true;
    }

    qWarning() << "Database connection failed:" << m_data.db.lastError().text();
    return false;
}

DataBaseManager::DataBaseManager()
{
    Config* pConf = Config::instance();
    if(pConf){
        m_data.db = QSqlDatabase::addDatabase(pConf->data().dmsql.driverType);
        m_data.db.setDatabaseName(pConf->data().dmsql.dbname);
        m_data.db.setPort(pConf->data().dmsql.port);
        m_data.db.setHostName(pConf->data().dmsql.host);
        m_data.db.setUserName(pConf->data().dmsql.user);
        m_data.db.setPassword(pConf->data().dmsql.password);

        //redis
    }

}
