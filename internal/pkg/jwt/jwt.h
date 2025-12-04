// pkg/jwt/jwt.h
#pragma once
#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <QVector>
#include <QJsonArray>
#include <QVariant>

#include <jwt-cpp/jwt.h>

#include "../../conf/conf.h"

namespace qKratos::JWT {

struct Claims {
    QString sub;           // 用户ID
    QString name;
    QStringList roles;
    qint64 exp = QDateTime::currentDateTimeUtc().addSecs(Config::instance()->auth().expires).toSecsSinceEpoch(); // expires为3天有效期
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

QString sign(const Claims& claims);     // 签发
QVariant verify(const QString& token); // 验证


//-----------------集成jwt-cpp---------------------
class JwtHelper {
public:
    // 签发 Token
    static QString sign(const Claims& claims);

    // 验证 Token (验证失败返回空的 QVariant，成功返回包含 Claims 的 QVariant)
    static QVariant verify(const QString& tokenStr);
};

} // namespace qratos::jwt

Q_DECLARE_METATYPE(qKratos::JWT::Claims)

