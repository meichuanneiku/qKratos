#include "user_route.h"
#include "../../../internal/service/user/user_service.h"

void RegisterUserServiceRoutes(QHttpServer& server, UserServiceImpl* impl)
{
    server.route("/api/v1/users", QHttpServerRequest::Method::Post,
                 [impl](const QHttpServerRequest& req) {
                     return impl->CreateUser(req);
                 });

    server.route("/api/v1/users", QHttpServerRequest::Method::Get,
                 [impl](const QHttpServerRequest& req) {
                     return impl->ListUsers(req);
                 });

    server.route("/api/v1/users/<arg>",QHttpServerRequest::Method::Get,
                 [&] (quint64 id, const QHttpServerRequest &request) {
        QByteArray systemId = request.headers().value("systemid").toByteArray();
        return impl->GetUserByIdDirect(systemId.toInt(), QString::number(id));
    });

    server.route("/api/v1/users/<arg>", QHttpServerRequest::Method::Put,
                 [&] (quint64 id, const QHttpServerRequest& request) {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            return Status(InvalidParams);
        }
        QJsonObject obj = doc.object();
        obj["YHID"] = static_cast<qint64>(id);
        QByteArray systemId = request.headers().value("systemid").toByteArray();
        if (!impl->UpdateUserById(systemId.toInt(), id, obj)) {
            return Status(UserNotFound);
        }
        return JsonResponse(obj);
    });

    // /v1/user?id=
    server.route("/api/v1/user", QHttpServerRequest::Method::Get,
                  [impl](const QHttpServerRequest &request) {
        return impl->GetUserByIdDirect(request);
    } );

    // 登录接口：POST /api/v1/login
    server.route("/api/v1/login", QHttpServerRequest::Method::Post,
                 [impl](const QHttpServerRequest& request) {
         QByteArray systemId = request.headers().value("systemId").toByteArray();
        return impl->Login(systemId.toInt(), request);
    });

    //获取用户列表接口：POST /api/v1/user/list
    server.route("/api/v1/user/list", QHttpServerRequest::Method::Post,
                 [impl](const QHttpServerRequest& request) {
         QByteArray systemId = request.headers().value("systemId").toByteArray();
        return impl->UserList(systemId.toInt(), request);
    });
}
