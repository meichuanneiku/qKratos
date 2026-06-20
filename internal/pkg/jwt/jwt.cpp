// pkg/jwt/jwt.cpp
#include "jwt.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QStringList>
#include <QDebug>

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

QString JwtHelper::sign(const Claims &claims)
{
    // 1. 创建构建器
    auto builder = jwt::create()
        .set_type("JWS")
        // 设置标准 Claim: sub (Subject)
        .set_payload_claim("sub", jwt::claim(claims.sub.toStdString()))
        // 设置自定义 Claim: name
        .set_payload_claim("name", jwt::claim(claims.name.toStdString()));

    // 2. 处理 Roles (QStringList -> std::vector/set)
    // jwt-cpp 需要 std 容器来生成 JSON 数组
    std::vector<std::string> rolesStd;
    for (const auto& role : claims.roles) {
        rolesStd.push_back(role.toStdString());
    }
    // 将 vector 转为 claim (picojson 数组)
    builder.set_payload_claim("roles", jwt::claim(std::begin(rolesStd), std::end(rolesStd)));

    // 3. 处理时间 (qint64 -> std::chrono::system_clock::time_point)
    // jwt-cpp 严格依赖 chrono 进行时间校验
    auto iatTime = std::chrono::system_clock::from_time_t(claims.iat);
    auto expTime = std::chrono::system_clock::from_time_t(claims.exp);

    builder.set_issued_at(iatTime)
           .set_expires_at(expTime);

    // 4. 签名
    // 使用 HS256 算法和密钥
    const QString secretKey = Config::instance()->auth().jwtKey;
    std::string token = builder.sign(jwt::algorithm::hs256{secretKey.toStdString()});

    return QString::fromStdString(token);
}

QVariant JwtHelper::verify(const QString &tokenStr)
{
    try {
        // 1. 解码 Token
        auto decoded = jwt::decode(tokenStr.toStdString());

        // 2. 创建验证器
        const QString secretKey = Config::instance()->auth().jwtKey;
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secretKey.toStdString()});
            // .with_issuer("auth0"); // 如果 sign 时设置了 issuer，这里也要加上

        // 3. 执行验证 (签名错误或过期会抛出异常)
        verifier.verify(decoded);

        // 4. 验证成功，提取数据回填 Claims 结构体
        Claims claims;

        // 提取 sub
        if (decoded.has_payload_claim("sub"))
            claims.sub = QString::fromStdString(decoded.get_payload_claim("sub").as_string());

        // 提取 name
        if (decoded.has_payload_claim("name"))
            claims.name = QString::fromStdString(decoded.get_payload_claim("name").as_string());

        // 提取 roles (数组处理)
        if (decoded.has_payload_claim("roles")) {
            auto rolesClaim = decoded.get_payload_claim("roles");
            // jwt-cpp 默认使用 picojson，需要按 array 迭代
            auto rolesArray = rolesClaim.as_array();
            for (const auto& r : rolesArray) {
                // get<std::string>() 是 picojson 的用法
                claims.roles.append(QString::fromStdString(r.get<std::string>()));
            }
        }

        // 提取时间 (std::chrono -> qint64)
        auto iatTime = decoded.get_issued_at();
        auto expTime = decoded.get_expires_at();
        claims.iat = std::chrono::system_clock::to_time_t(iatTime);
        claims.exp = std::chrono::system_clock::to_time_t(expTime);

        // 5. 返回包装好的 QVariant
        return QVariant::fromValue(claims);

    } catch (const std::exception& e) {
        // 验证失败 (过期、签名不对、格式错误等)
        qDebug() << "JWT Verify Failed:" << e.what();
        return QVariant(); // 返回空 Variant 表示失败
    }
}

} // namespace
