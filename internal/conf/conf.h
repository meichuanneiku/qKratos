#ifndef CONF_H
#define CONF_H


#include <QString>
#include <QList>

struct ServerConfig {
    struct Server {
        QString addr;
        int     port;
        int     timeout;
    };

    Server http;
    Server ws;

};

// Data配置结构
struct DataConfig {
    struct DateBase {
        QString host;
        int port;
        QString user;
        QString password;
        QString dbname;
    };

    struct DmSQL {
        QString host;
        int port;
        QString user;
        QString password;
        QString dbname;
        QString driverType;
    };

    struct Redis {
        QString addr;
        QString password;
        int db;
    };

    DateBase mysql;
    DateBase pgsql;
    DmSQL dmsql;
    Redis redis;
};

struct AuthConfig{
    QString jwtKey;
    qint64 expires;
};

struct Cron{
    QString jobName;
    QString cronExpr;
};

struct Log{
    QString logDir;
    QString prefix;
    qint64 maxSize;
    QString logLevel;
    bool consoleOutput;
    bool async;
};

struct TableConfig {
    QString user;
    QString user2;
    QString role;
    QString role2;
    QString fun;
    QString fun2;
    QString fxtb;
};

class Config
{
public:
    static Config* instance();

    bool load(const QString& inputPath = "configs/config.yaml");
    const ServerConfig& server() const { return m_server; }
    const DataConfig& data() const { return m_data; }
    const QString& casbinPath() const { return m_casbinPath; }
    const AuthConfig& auth() const { return m_auth; }
    const TableConfig& tables() const { return m_tables; }


    const Log& log() const { return m_log; }

    const QList<Cron>& cronList() const { return m_cronList; }
private:
    static Config* m_instance;
    Config() = default;
    ServerConfig m_server;
    DataConfig m_data;
    QString m_casbinPath;
    AuthConfig m_auth;
    Log m_log;
    QList<Cron> m_cronList;
    TableConfig m_tables;
};
#endif // CONF_H
