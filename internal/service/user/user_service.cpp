#include "user_service.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "../../pkg/middleware/auth.h"

using namespace qKratos::Middleware;

QHttpServerResponse UserServiceImpl::CreateUser(const QHttpServerRequest& req)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(req.body(), &err);
    if (err.error != QJsonParseError::NoError){
        return Status(InvalidParams);
    }

    QString name = doc.object()["name"].toString();
    if (name.isEmpty())
        return Status(UserNameEmpty);

    int id = m_biz.CreateUser(name);

    QJsonObject body{ {"id", id}, {"name", name} };
    return JsonResponse(body);
}

QHttpServerResponse UserServiceImpl::GetUser(const int &systemId, const QRegularExpressionMatch& match)
{
    bool ok;
    int id = match.captured(1).toInt(&ok);
    if (!ok) return Status(UserIdInvalid);

    auto user = m_biz.GetUser(systemId, QString::number(id));
    if (user.isEmpty()) return Status(UserNotFound);

    return JsonResponse(user);
}

QHttpServerResponse UserServiceImpl::DeleteUser(const QRegularExpressionMatch& match)
{
    bool ok;
    int id = match.captured(1).toInt(&ok);
    if (!ok) return Status(UserIdInvalid);

    if(!m_biz.DeleteUser(id)){

        return Status(UserNotFound);
    }
    return Status();
}

QHttpServerResponse UserServiceImpl::GetUserByIdDirect(const int &systemId, const QString &id)
{
    auto user = m_biz.FindById(systemId, id);
    if (user.isEmpty())
        return Status(UserNotFound);

    return JsonResponse(user);
}

QHttpServerResponse UserServiceImpl::DeleteUser(const int &id)
{
    m_biz.DeleteUser(id);
    return Status();
}

QHttpServerResponse UserServiceImpl::GetUserByIdDirect(const QHttpServerRequest &request)
{
    QUrlQuery query  = request.query();
   if(!query.hasQueryItem("id")){
        return Status(UserIdEmpty);
   }
   QString id = query.queryItemValue("id");

   QByteArray systemId = request.headers().value("systemid").toByteArray();
   return GetUserByIdDirect(systemId.toInt(), id);
}

QHttpServerResponse UserServiceImpl::GetProfile(const QHttpServerRequest &request)
{
    QVariant userVar = currentUser();
    if (!userVar.isValid()) {
        return Status(Unauthorized);
    }

    auto claims = userVar.value<Claims>();
    QJsonObject data{
        {"id", claims.sub},
        {"name", claims.name},
        {"roles", QJsonArray::fromStringList(claims.roles)}
    };

    return JsonResponse(data);
}

QHttpServerResponse UserServiceImpl::Login(const int &systemId, const QHttpServerRequest &request)
{
    // 1. 解析请求体
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return Status(InvalidParams);  // 自定义错误码：参数错误
    }

    QJsonObject obj = doc.object();
    QString username = obj["username"].toString();
    QString password = obj["password"].toString();

    if(username.isEmpty() || password.isEmpty()){
         return Status(Unauthorized);   // 账号或密码错误
    }
/*
    // 2. 简单的账号密码校验（实际项目请查数据库 + 加密比对）
    if (username != "admin" || password != "123456") {
        return Status(Unauthorized);   // 账号或密码错误
    }

    // 3. 签发 JWT
    Claims claims;
    claims.sub = "1001";                    // 用户ID
    claims.name = "管理员";
    claims.roles = QStringList() <<"admin" << "user";

//    QString token = sign(claims);
    QString token = JwtHelper::sign(claims);

    // 4. 返回 token + 用户信息
    QJsonObject data{
        {"token", token},
        {"user_id", claims.sub},
        {"name",    claims.name},
        {"roles",   QJsonArray::fromStringList(claims.roles)}
    };

    return JsonResponse(data);  // 自动包装成统一格式*/
    auto login = m_biz.UserLogin(systemId, username, password);
    if (login.isEmpty()) return Status(UserNotFound);

    return JsonResponse(login);

}
