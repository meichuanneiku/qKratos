#ifndef DATA_H
#define DATA_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

struct Data {
    QSqlDatabase db;
};

class DataBaseManager
{
public:

    ~DataBaseManager();

    static DataBaseManager *instance();

    bool connect();

    const Data& data() const { return m_data; }

    const QString dbError() const { return m_data.db.lastError().text(); }
private:
    DataBaseManager();


    static DataBaseManager* m_instance;

private:
    Data m_data;
};

#endif // DATA_H
