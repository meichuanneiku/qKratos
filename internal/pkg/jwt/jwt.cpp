// pkg/jwt/jwt.cpp
#include "jwt.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QStringList>

namespace qKratos::JWT {

static QString base64UrlEncode(const QByteArray& data) {
    return data.toBase64()
            .replace('+', '-')
            .replace('/', '_')
            .replace("=", "");
}

static QByteArray base64UrlDecode(const QString& input) {
    QByteArray data = input.toLatin1();
    data.replace('-', '+');
    data.replace('_', '/');
    while (data.size() % 4) data.append('=');
    return QByteArray::fromBase64(data);
}

QString sign(const Claims& claims)
{
    const QString secret = Config::instance()->auth().jwtKey;
    QJsonObject header{{"alg","HS256"},{"typ","JWT"}};
    QJsonObject payload = claims.toJson();

    QString encodedHeader = base64UrlEncode(QJsonDocument(header).toJson(QJsonDocument::Compact));
    QString encodedPayload = base64UrlEncode(QJsonDocument(payload).toJson(QJsonDocument::Compact));

    QString unsignedToken = encodedHeader + "." + encodedPayload;
    QByteArray signature = QCryptographicHash::hash(unsignedToken.toUtf8() + secret.toUtf8(), QCryptographicHash::Sha256);

    return unsignedToken + "." + base64UrlEncode(signature);
}

QVariant verify(const QString& token)
{    
    const QString& secret = Config::instance()->auth().jwtKey;

    QStringList parts = token.split('.');
    if (parts.size() != 3) return QVariant();

    QString unsignedToken = parts[0] + "." + parts[1];
    QByteArray expectedSig = QCryptographicHash::hash(unsignedToken.toUtf8() + secret.toUtf8(), QCryptographicHash::Sha256);
    QByteArray actualSig = base64UrlDecode(parts[2]);

    if (expectedSig != actualSig) return QVariant();

    QJsonDocument doc = QJsonDocument::fromJson(base64UrlDecode(parts[1]));
    if (doc.isNull()) return QVariant();

    QJsonObject payload = doc.object();
    qint64 exp = payload["exp"].toVariant().toLongLong();

    if (QDateTime::currentDateTimeUtc().toSecsSinceEpoch() > exp)
        return QVariant(); // 过期

    Claims claims;
    claims.sub = payload["sub"].toString();
    claims.name = payload["name"].toString();
    auto arr = payload["roles"].toArray();
    for (const auto& v : arr) {
        claims.roles << v.toString();
    }

    return QVariant::fromValue(claims);   // 成功！
}

} // namespace
