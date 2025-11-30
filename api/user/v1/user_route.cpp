#include "user_route.h"

#include "../../../internal/service/user/user_service.h"

static inline QString host(const QHttpServerRequest &request)
{
    return request.headers()[QStringLiteral("Host")].toString();
}

void RegisterUserServiceRoutes(QHttpServer& server, UserServiceImpl* impl)
{
    server.route("/api/v1/users", QHttpServerRequest::Method::Post,
                 [impl](const QHttpServerRequest& req) {
                     return impl->CreateUser(req);
                 });

    server.route("/api/v1/users/<arg>",QHttpServerRequest::Method::Get,
                 [&] (quint64 id, const QHttpServerRequest &request) {
        QByteArray systemId = request.headers().value("systemid").toByteArray();
        return impl->GetUserByIdDirect(systemId.toInt(), QString::number(id));
    });

    // /v1/user?id=
    server.route("/api/v1/user", QHttpServerRequest::Method::Get,
                  [impl](const QHttpServerRequest &request) {
        return impl->GetUserByIdDirect(request);
    } );

    // 登录接口：POST /api/v1/login
    server.route("/api/v1/login", QHttpServerRequest::Method::Post,
                 [impl](const QHttpServerRequest& request) {
        return impl->Login(request);
    });
}
