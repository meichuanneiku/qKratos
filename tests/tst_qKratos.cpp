#include <QtTest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "../internal/pkg/jwt/jwt.h"
#include "../internal/pkg/response/response.h"
#include "../internal/pkg/error/error_code.h"

using namespace qKratos::JWT;
using namespace qKratos::Response;
using namespace qKratos::Error;

class TestqKratos : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void testJwtSignAndVerify();
    void testJwtExpired();
    void testJwtInvalidToken();
    void testJsonResponse();
    void testStatusResponse();
    void testErrorCodeValues();
};

void TestqKratos::initTestCase()
{
    qRegisterMetaType<Claims>();
}

void TestqKratos::testJwtSignAndVerify()
{
    Claims claims;
    claims.sub = "1001";
    claims.name = "测试用户";
    claims.roles = QStringList() << "admin" << "user";

    QString token = JwtHelper::sign(claims);
    QVERIFY(!token.isEmpty());
    QVERIFY(token.count('.') == 2);

    QVariant result = JwtHelper::verify(token);
    QVERIFY(result.isValid());

    Claims verified = result.value<Claims>();
    QCOMPARE(verified.sub, claims.sub);
    QCOMPARE(verified.name, claims.name);
    QCOMPARE(verified.roles, claims.roles);
}

void TestqKratos::testJwtExpired()
{
    Claims claims;
    claims.sub = "1002";
    claims.name = "过期用户";
    claims.exp = QDateTime::currentDateTimeUtc().addSecs(-3600).toSecsSinceEpoch();

    QString token = JwtHelper::sign(claims);

    QVariant result = JwtHelper::verify(token);
    QVERIFY(!result.isValid());
}

void TestqKratos::testJwtInvalidToken()
{
    QVariant result = JwtHelper::verify("invalid.token.string");
    QVERIFY(!result.isValid());

    result = JwtHelper::verify("eyJhbGciOiJIUzI1NiJ9.dGVzdA.tampered");
    QVERIFY(!result.isValid());

    result = JwtHelper::verify("");
    QVERIFY(!result.isValid());
}

void TestqKratos::testJsonResponse()
{
    QJsonObject data;
    data["key"] = "value";

    QHttpServerResponse resp = JsonResponse(data);
    QCOMPARE(static_cast<int>(resp.statusCode()), 200);

    QJsonDocument doc = QJsonDocument::fromJson(resp.data());
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object()["key"].toString(), QString("value"));
}

void TestqKratos::testStatusResponse()
{
    QHttpServerResponse resp = Status(UserNameEmpty);
    QVERIFY(static_cast<int>(resp.statusCode()) >= 1000);

    resp = Status(QHttpServerResponse::StatusCode::Ok);
    QCOMPARE(static_cast<int>(resp.statusCode()), 200);
}

void TestqKratos::testErrorCodeValues()
{
    QCOMPARE(InvalidParams, 1000);
    QCOMPARE(UserNameEmpty, 1001);
    QCOMPARE(PasswordTooShort, 1002);
    QCOMPARE(EmailInvalid, 1003);
    QCOMPARE(UserIdEmpty, 1004);
    QCOMPARE(UserNotFound, 1005);
    QCOMPARE(UserIdInvalid, 1006);
    QCOMPARE(Unauthorized, 1007);
}

QTEST_APPLESS_MAIN(TestqKratos)
#include "tst_qKratos.moc"
