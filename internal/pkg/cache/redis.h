#pragma once

#include "cache.h"
#include <QTcpSocket>
#include <QHostAddress>

namespace qKratos::Cache {

class RedisCache : public Cache {
public:
    explicit RedisCache(const QString& host = "127.0.0.1", quint16 port = 6379,
                        const QString& password = "", int db = 0);
    ~RedisCache() override;

    bool connect();
    void disconnect();

    bool set(const QString& key, const QByteArray& value, int ttlSeconds = 0) override;
    QByteArray get(const QString& key) override;
    bool del(const QString& key) override;
    bool exists(const QString& key) override;
    bool isConnected() const override;

private:
    QByteArray buildCommand(const QByteArray& cmd, const QList<QByteArray>& args);
    QByteArray readReply();
    bool sendCommand(const QByteArray& data);

    QTcpSocket* m_socket;
    QString m_host;
    quint16 m_port;
    QString m_password;
    int m_db;
    bool m_connected;
};

} // namespace qKratos::Cache
