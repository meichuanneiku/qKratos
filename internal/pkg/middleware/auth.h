#pragma once

#include <QtHttpServer/qhttpserverresponse.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QVariant>
#include <QThreadStorage>

#include "../jwt/jwt.h"

//#include "../error/error_code.h"
#include "../casbin/casbin_dm_adapter.h"

#include "../response/response.h"

using namespace qKratos::response;
using namespace qKratos::JWT;
//using namespace qKratos::Error;
using namespace qKratos::rbac;

namespace qKratos::middleware {


inline QThreadStorage<QVariant>& currentUserStorage()
{
    static QThreadStorage<QVariant> storage;
    return storage;
}

// 业务层直接调用
inline QVariant currentUser()
{
    return currentUserStorage().localData();   // 自动返回 QVariant()
}

// 设置当前用户
inline void setCurrentUser(const QVariant& user)
{
    currentUserStorage().setLocalData(user);
}

// 工具函数：Method → QString
inline QString methodToString(QHttpServerRequest::Method m) {
    switch (m) {
    case QHttpServerRequest::Method::Get:     return "GET";
    case QHttpServerRequest::Method::Post:    return "POST";
    case QHttpServerRequest::Method::Put:     return "PUT";
    case QHttpServerRequest::Method::Delete:  return "DELETE";
    case QHttpServerRequest::Method::Head:    return "HEAD";
    case QHttpServerRequest::Method::Options: return "OPTIONS";
    case QHttpServerRequest::Method::Patch:   return "PATCH";
    case QHttpServerRequest::Method::Connect: return "CONNECT";
    default:                                  return "UNKNOWN";
    }
}

// JWT 中间件（inline 实现，保持简洁）
inline auto authMiddleware()
{
    return [](QHttpServerResponse&& resp, const QHttpServerRequest& req) -> QHttpServerResponse {
            QString path = req.url().path();

            // 公开接口清空
            if (path == "/api/v1/login" || path.startsWith("/api/v1/public/")) {
                setCurrentUser(QVariant());
                return std::move(resp);
            }

            QString auth = req.value("Authorization");
            if (!auth.startsWith("Bearer ", Qt::CaseInsensitive)) {
                return Status(ErrorCode::Unauthorized);
            }

            QString token = auth.mid(7);
            QVariant result = verify(token);
            if (!result.isValid()) {
                return Status(ErrorCode::Unauthorized);
            }

            setCurrentUser(result);

            // Casbin 权限判断
            Claims claims = result.value<Claims>();
            QString userId = claims.sub;
            QString method = methodToString(req.method());

            if (!enforce(userId, path, method)) {
                return Status(ErrorCode::Forbidden);
            }

            return std::move(resp);
        };
}

} // namespace
