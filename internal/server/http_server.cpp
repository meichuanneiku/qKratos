#include "http_server.h"

#include "../pkg/errorinfo/error_info.h"
#include "../pkg/response/response.h"
#include "../pkg/middleware/auth.h"
#include <QDebug>

using namespace qKratos::response;
using namespace qKratos::middleware;

HttpServer::HttpServer(const int &timeout)
{
    m_server.afterRequest([timeout](QHttpServerResponse&& resp, const QHttpServerRequest&req) {
            resp.setHeader("X-Powered-By", "QKratos/1.0");
            // 允许的源，* 表示允许所有源
            resp.setHeader("Access-Control-Allow-Origin", "*");
            // 允许的请求头
            resp.setHeader("Access-Control-Allow-Headers", "*");
            // 允许的请求方法
            resp.setHeader("Access-Control-Allow-Methods", "*");
            // 预检请求的有效期（秒）
            resp.setHeader("Access-Control-Max-Age", QString::number(timeout).toUtf8());


            QHttpServerResponse r = std::move(resp);
            r = authMiddleware("my-secret-2025")(std::move(r), req);  // Auth
//            r = loggingMiddleware(std::move(r), req);  // Logging
            r = responseMiddleware(std::move(r), req);  // 统一格式 + Recovery
            return r;



    });
}

bool HttpServer::listen(const QString &address, const quint16 &port)
{
    if (m_server.listen(QHostAddress(address), port)) {
        qDebug() << QString(" Running on http://%2:%1").arg(port).arg(address);
        return true;
    }
    return false;
}

QHttpServerResponse HttpServer::responseMiddleware(QHttpServerResponse &&r, const QHttpServerRequest &req)
{
    // Recovery: try-catch 异常处理
    try {

        int status = static_cast<int>(r.statusCode());

        // 关键：检测是否是我们用 Error() 返回的“标记响应”
        if (status >= 600 && status < 700) {
            // 已经是统一格式了，直接返回！（零开销）
            return std::move(r);
        }

        // 普通响应：统一包装
        QJsonObject unified;
        unified["code"] = (status >= 200 && status < 300) ? 0 : status;
        unified["message"] = getMessage(status);

        // 尝试解析原始 body
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(r.data(), &err);
        QJsonObject origData = doc.object();
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            unified["data"] = origData;
        } else {
            unified["data"] = QJsonObject{};
        }

        return JsonResponse(unified, r.statusCode());  // 保留原始 HTTP 状态码

    } catch (...) {
//        Logger::log(Logger::Error, "Response processing failed");
        return QHttpServerResponse(QHttpServerResponse::StatusCode::InternalServerError);
    }
}

QString HttpServer::getMessage(int status)
{
    // 只有原始 HTTP 错误才走这里（比如框架抛的 404、500）
        // 所有 Error(UserNotFound) 已经自带 message，直接放行了！
        if (status >= 200 && status < 300)
            return QStringLiteral("成功");

        switch (status) {
        case 400: return QStringLiteral("请求参数错误");
        case 401: return QStringLiteral("未授权");
        case 403: return QStringLiteral("禁止访问");
        case 404: return QStringLiteral("资源不存在");
        case 405: return QStringLiteral("方法不允许");
        case 500: return QStringLiteral("服务器错误");
        default:  return QStringLiteral("未知错误");
        }
}
