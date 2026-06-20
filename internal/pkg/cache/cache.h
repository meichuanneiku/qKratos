#pragma once

#include <QString>
#include <QByteArray>
#include <QVariant>

namespace qKratos::Cache {

class Cache {
public:
    virtual ~Cache() = default;

    virtual bool set(const QString& key, const QByteArray& value, int ttlSeconds = 0) = 0;
    virtual QByteArray get(const QString& key) = 0;
    virtual bool del(const QString& key) = 0;
    virtual bool exists(const QString& key) = 0;
    virtual bool isConnected() const = 0;
};

} // namespace qKratos::Cache
