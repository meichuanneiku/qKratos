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

    auto login = m_biz.UserLogin(systemId, username, password);

    if (!login.isSuccess()) return Status(login.errorCode);

    return JsonResponse(login.data);
}

QHttpServerResponse UserServiceImpl::UserList(const int &systemId, const QHttpServerRequest &request)
{
    // 1. 解析请求体
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return Status(InvalidParams);  // 自定义错误码：参数错误
    }

    QJsonObject obj = doc.object();

    QString fxtid         = obj["FXTID"].toString();
    QString jsid          = obj["JSID"].toString();
    QString zxzt          = obj["ZXZT"].toString();
    QString yhmc          = obj["YHMC"].toString();
    QString dlcsCondition = obj["DLCSCondition"].toString();
    int dlcs              = obj["DLCS"].toInt(-1);
    int page              = obj["page"].toInt();
    int pageSize          = obj["pageSize"].toInt();

    //判断必填字段
   /* if(dlcs == -1){
        return Status(InvalidParams);  // 自定义错误码：参数错误
    }*/


    auto userList = m_biz.UserList(systemId, fxtid, jsid, zxzt, yhmc, dlcsCondition, dlcs, page, pageSize);

    if (!userList.isSuccess()) return Status(userList.errorCode);

    return JsonResponse(userList.data);

}

QJsonObject UserServiceImpl::pollAndPush(const QJsonObject &doc)
{
    int SJJG = doc["params"]["SJJG"].toInt();
    //    auto xhdIdArr = doc["params"]["XHDIDLB"].toArray();
    QStringList xhdIdList;
    for (const auto &val : doc["params"]["XHDIDLB"].toArray()) {
        xhdIdList << val.toString();
    }
//    QString currentTime =  QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");



//    auto userList = m_biz.pollAndPush();
    return {};

}
