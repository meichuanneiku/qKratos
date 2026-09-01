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

    QByteArray systemId = req.headers().value("systemid").toByteArray();
    int id = m_biz.CreateUser(systemId.toInt(), name);

    QJsonObject body{ {"id", id}, {"name", name} };
    return JsonResponse(body);
}

QHttpServerResponse UserServiceImpl::ListUsers(const QHttpServerRequest& request)
{
    QByteArray systemId = request.headers().value("systemid").toByteArray();
    QUrlQuery query = request.query();
    QString page = query.queryItemValue("page");
    if (page.isEmpty()) page = "0";

    return ListUsers(systemId.toInt(), page);
}

QHttpServerResponse UserServiceImpl::ListUsers(const int &systemId, const QString &page)
{
    auto result = m_biz.ListUsers(systemId, page);
    return JsonResponse(result);
}

QHttpServerResponse UserServiceImpl::UpdateUserById(const int& systemId, quint64 id, const QJsonObject& data)
{
    QJsonObject obj = data;
    obj["YHID"] = static_cast<qint64>(id);
    if (!m_biz.UpdateUser(systemId, obj)) {
        return Status(UserNotFound);
    }
    return JsonResponse(obj);
}

QHttpServerResponse UserServiceImpl::UpdateUser(const QHttpServerRequest& request)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return Status(InvalidParams);
    }

    QByteArray systemId = request.headers().value("systemid").toByteArray();
    QJsonObject obj = doc.object();
    if (!obj.contains("YHID")) return Status(UserIdEmpty);

    if (!m_biz.UpdateUser(systemId.toInt(), obj)) {
        return Status(UserNotFound);
    }
    return JsonResponse(obj);
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

    // 路由层暂不传递 systemId，使用默认值 1
    if(!m_biz.DeleteUser(1, id)){
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
    m_biz.DeleteUser(1, id);
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

    return Login(systemId, username, password);
}

QHttpServerResponse UserServiceImpl::Login(const int &systemId, const QString &username, const QString &password)
{
    if(username.isEmpty() || password.isEmpty()){
         return Status(Unauthorized);
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

    return UserList(systemId,
                    obj["FXTID"].toString(),
                    obj["JSID"].toString(),
                    obj["ZXZT"].toString(),
                    obj["YHMC"].toString(),
                    obj["DLCSCondition"].toString(),
                    obj["DLCS"].toInt(-1),
                    obj["page"].toInt(),
                    obj["pageSize"].toInt());
}

QHttpServerResponse UserServiceImpl::UserList(const int &systemId, const QString &fxtid, const QString &jsid,
                                              const QString &zxzt, const QString &yhmc,
                                              const QString &dlcsCondition, const int &dlcs,
                                              const int &page, const int &pageSize)
{
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
