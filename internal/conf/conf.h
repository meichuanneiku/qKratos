#ifndef CONF_H
#define CONF_H


#include <QString>

enum Env
{
  dev = 0, // 开发环境
  test = 1, // 测试环境
  pro = 2 // 正式环境
};

struct ServerConfig {
    struct Server {
        QString addr;
        int     port;
        int     timeout;
    };

    Server http;
    Server grpc;
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
    DmSQL dmsql;
    Redis redis;
};

struct AuthConfig{
    QString jwtKey;
    qint64 expires;
};

class Config
{
public:
    static Config* instance();
    /* {
        static Config c;
        return c;
    }*/

    bool load(const QString& inputPath = "configs/config.yaml");
    const ServerConfig& server() const { return m_server; }
    const DataConfig& data() const { return m_data; }
    const QString& casbinPath() const { return m_casbinPath; }
    const AuthConfig& auth() const { return m_auth; }

private:
    static Config* m_instance;
    Config() = default;
    ServerConfig m_server;

    DataConfig m_data;
    QString m_casbinPath;
    AuthConfig m_auth;
};
#endif // CONF_H
