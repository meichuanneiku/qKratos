#include "http_server.h"

#include "../pkg/errorinfo/error_info.h"

#include <QDebug>
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
//            return std::move(resp);

            QHttpServerResponse r = std::move(resp);
    //        r = authMiddleware(std::move(r), req);  // Auth
//            r = loggingMiddleware(std::move(r), req);  // Logging
            r = responseMiddleware(std::move(r), req);  // 统一格式 + Recovery
            return r;
    });
}

bool HttpServer::listen(const QString &address, const quint16 &port)
{
    if (m_server.listen(QHostAddress(address), port)) {
        qDebug() << QString("QHttpServerExample Running on http://127.0.0.1:%1").arg(port);
        return true;
    }
    return false;
}

QHttpServerResponse HttpServer::responseMiddleware(QHttpServerResponse &&r, const QHttpServerRequest &req)
{
    // Recovery: try-catch 异常处理
    try {
        // 统一结构
        QJsonObject unified;


        /*// 检查 Content-Type
        const QVector<QByteArray> ctValues = r.headers("Content-Type");
        bool isJson = false;
        for (const QByteArray &v : ctValues) {
            if (v.compare("application/json", Qt::CaseInsensitive) == 0) {
                isJson = true;
                break;
            }
        }

        if (!isJson) {
            return std::move(r);
        }*/

        if(r.statusCode() == QHttpServerResponse::StatusCode::Ok){
            // 解析 body
            QJsonParseError parseErr{};
            QJsonDocument doc = QJsonDocument::fromJson(r.data(), &parseErr);
            if (parseErr.error != QJsonParseError::NoError) {
                return std::move(r);
            }

            QJsonObject origData = doc.object();


            unified["code"] = static_cast<int>(r.statusCode());
            unified["message"] = getMessage(static_cast<int>(r.statusCode()), origData.value("message").toString());
            unified["data"] = origData;

        }else if (r.statusCode() == QHttpServerResponse::StatusCode::NoContent){
            const auto& registry = getErrorRegistry();
            auto it = registry.find(r.data().toInt());
            unified["code"] = it->code;
            unified["message"] = it->message;
            unified["data"] = QJsonObject();
        }

        QJsonDocument newDoc(unified);
        QByteArray newBody = newDoc.toJson(QJsonDocument::Compact);

        // 构建新响应（保留 status，替换 body/mime）
        QHttpServerResponse newResp("application/json", newBody);
        return newResp;
    } catch (...) {
//        Logger::log(Logger::Error, "Response processing failed");
        return QHttpServerResponse(QHttpServerResponse::StatusCode::InternalServerError);
    }
}

QString HttpServer::getMessage(int code, const QString &defaultMsg)
{
    if (code >= 200 && code < 300) return QStringLiteral("成功");
    if (defaultMsg.isEmpty()) return QString("错误码: %1").arg(code);
    return defaultMsg;
}
