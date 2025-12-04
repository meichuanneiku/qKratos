#include "http_server.h"
#include "../pkg/middleware/auth.h"


using namespace qKratos::Middleware;

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
            r = authMiddleware()(std::move(r), req);  // Auth
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

        bool isCustomErrorCode = status >= 1000;

        // 普通响应：统一包装
        QJsonObject unified;
        unified["code"] = status;
        unified["message"] = getMessage(status);

        // 尝试解析原始 body
        if(isCustomErrorCode){
            unified["data"] = QJsonObject{};
        }
        else{
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(r.data(), &err);
            QJsonObject origData = doc.object();
            if (err.error == QJsonParseError::NoError && doc.isObject()) {
                unified["data"] = origData;
            } else {
                unified["data"] = QJsonObject{};
            }
        }

        return JsonResponse(unified, isCustomErrorCode ?  QHttpServerResponse::StatusCode::Ok : r.statusCode());  // 保留原始 HTTP 状态码

    } catch (...) {
//        Logger::log(Logger::Error, "Response processing failed");
        return QHttpServerResponse(QHttpServerResponse::StatusCode::InternalServerError);
    }
}

QString HttpServer::getMessage(int status)
{
    switch (status) {
    case 200: return QStringLiteral("成功");
    case 400: return QStringLiteral("请求参数错误");
    case 401: return QStringLiteral("未授权");
    case 403: return QStringLiteral("禁止访问");
    case 404: return QStringLiteral("资源不存在");
    case 405: return QStringLiteral("方法不允许");
    case 500: return QStringLiteral("服务器错误");

    case UserNameEmpty:     return "用户名不能为空";
    case UserNotFound:      return "用户不存在";
    case Unauthorized:      return "认证数据不可为空";
    default:  return QStringLiteral("未知错误");
    }
}

