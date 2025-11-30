// pkg/jwt/jwt.h
#pragma once
#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <QVector>
#include <QJsonArray>
#include <QVariant>

namespace qKratos::JWT {

struct Claims {
    QString sub;           // 用户ID
    QString name;
    QStringList roles;
    qint64 exp = QDateTime::currentDateTimeUtc().addSecs(3600 * 24 * 7).toSecsSinceEpoch(); // 7天有效期
    qint64 iat = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();

    QJsonObject toJson() const {
        {
        QJsonObject obj;
        obj["sub"] = sub;
        obj["name"] = name;
        obj["roles"] = QJsonArray::fromStringList(roles);
        obj["exp"] = exp;
        obj["iat"] = iat;
        return obj;
    }
    }
};

QString sign(const Claims& claims, const QString& secret);     // 签发
QVariant verify(const QString& token, const QString& secret); // 验证

} // namespace qratos::jwt

Q_DECLARE_METATYPE(qKratos::JWT::Claims)

