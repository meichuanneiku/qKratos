#include "http_server.h"
#include "../pkg/middleware/auth.h"
#include "../../cmd/server/appcontext.h"


using namespace qKratos::Middleware;

#if HTTPSERVER_INSTANCE
void HttpServer::init(const int &timeout)
{
    m_server.afterRequest([timeout](QHttpServerResponse&& resp, const QHttpServerRequest&req) {
<<<<<<< HEAD
=======
            resp.setHeader("X-Powered-By", "QKratos/1.0");
            // 允许的源，* 表示允许所有源
            resp.setHeader("Access-Control-Allow-Origin", "*");
            // 允许的请求头
            resp.setHeader("Access-Control-Allow-Headers", "*");
            // 允许的请求方法
            resp.setHeader("Access-Control-Allow-Methods", "*");
            // 预检请求的有效期（秒）
            resp.setHeader("Access-Control-Max-Age", "86400");
>>>>>>> 346b751 (audit: 代码审计修复 + 新功能)

        // 允许的源，* 表示允许所有源
        resp.setHeader("Access-Control-Allow-Origin", "*");
        // 允许的请求头
        resp.setHeader("Access-Control-Allow-Headers", "*");
        // 允许的请求方法
        resp.setHeader("Access-Control-Allow-Methods", "*");
        // 预检请求的有效期（秒）
        //            resp.setHeader("Access-Control-Max-Age", QString::number(timeout).toUtf8());

        QHttpServerResponse r = std::move(resp);

        r = authMiddleware()(std::move(r), req);  // Auth
        //        r = loggingMiddleware(std::move(r), req);  // Logging
        r = responseMiddleware(std::move(r), req);  // 统一格式 + Recovery
        return r;
    });

    connect(&m_server, &QHttpServer::newWebSocketConnection, this, &HttpServer::onNewConnection);
}

void HttpServer::initWebSocket()
{
    connect(&m_server, &QHttpServer::newWebSocketConnection, this, &HttpServer::onNewConnection);
}

#else
HttpServer::HttpServer(const int &timeout, QObject *parent): QObject(parent)
{
    m_server.afterRequest([timeout](QHttpServerResponse&& resp, const QHttpServerRequest&req) {

        // 允许的源，* 表示允许所有源
        resp.setHeader("Access-Control-Allow-Origin", "*");
        // 允许的请求头
        resp.setHeader("Access-Control-Allow-Headers", "*");
        // 允许的请求方法
        resp.setHeader("Access-Control-Allow-Methods", "*");
        // 预检请求的有效期（秒）
//        resp.setHeader("Access-Control-Max-Age", QString::number(timeout).toUtf8());

        QHttpServerResponse r = std::move(resp);

        r = authMiddleware()(std::move(r), req);  // Auth
        r = loggingMiddleware(std::move(r), req);  // Logging
        r = responseMiddleware(std::move(r), req);  // 统一格式 + Recovery
        return r;
    });

    connect(&m_server, &QHttpServer::newWebSocketConnection, this, &HttpServer::onNewConnection);
}
#endif

HttpServer::~HttpServer()
{
    // 释放所有客户端连接对象的内存
    m_webClientB.clear();
    qDeleteAll(m_webClients);
}

bool HttpServer::listen(const QString &address, const quint16 &port)
{
    if (!m_server.listen(QHostAddress(address), port)) {
        return false;
    }
    qDebug() << QString(" Running on http://%2:%1").arg(port).arg(address);
    return true;
}

void HttpServer::onNewConnection()
{
    // 获取新连接的 WebSocket 对象
    QWebSocket *pSocket = m_server.nextPendingWebSocketConnection();
    // 输出新连接的客户端的 IP 地址信息
    qDebug() << "New WebSocket connection:" << pSocket->peerAddress().toString();

    // 连接接收到文本消息信号到处理函数，
    //原则上接收到数据要放到具体的处理类中单独处理
    connect(pSocket, &QWebSocket::textMessageReceived, this, [=](const QString &message){
        // 输出接收到的消息内容
        qDebug() << "Received message2:" << message;
        //解析json，判断json中的type，调用具体的处理方法
        QJsonObject msg = QJsonDocument::fromJson(message.toUtf8()).object();
        if(msg["type"].toString() == "deviceParam"){

             // 仅添加新客户端，不修改已有数据
            m_webClientB = AppContext::instance().webSocketListB();
            if (!msg.contains("params")){
                return ;
            }

            if (msg["content"].toString() == "start"){
                QJsonObject params = msg["params"].toObject();
                if(!params.contains("index")){
                    params["index"] = 0;
                }
                m_webClientB.insert(pSocket, params);
            }
            else if(msg["content"].toString() == "close"){
                m_webClientB.remove(pSocket);
                m_webClients.removeOne(pSocket);
                pSocket->deleteLater();
            }

#if HTTPSERVER_INSTANCE

#else
            AppContext::instance().setWebSocketB(m_webClientB);
#endif

        }
    });

    // 连接客户端断开连接信号到处理函数
    connect(pSocket, &QWebSocket::disconnected, this, [=](){

        if(m_webClients.contains(pSocket)){
            m_webClients.removeOne(pSocket);
        }
        if(m_webClientB.contains(pSocket)){
            m_webClientB.remove(pSocket);
        }
        // 释放断开连接的客户端对象的内存
        pSocket->deleteLater();

#if HTTPSERVER_INSTANCE

#else
            AppContext::instance().setWebSocketB(m_webClientB);
#endif
    });

    m_webClients << pSocket;
}

QHttpServerResponse HttpServer::responseMiddleware(QHttpServerResponse &&r, const QHttpServerRequest &req)
{
    try {
        int status = static_cast<int>(r.statusCode());
        bool isCustomErrorCode = status >= 1000;

        QJsonObject unified;
        unified["code"] = isCustomErrorCode ? status : 0;
        unified["message"] = getMessage(status);

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(r.data(), &err);
        if (err.error == QJsonParseError::NoError && doc.isObject()) {
            unified["data"] = doc.object();
        } else {
            unified["data"] = QJsonObject{};
        }

        // 自定义错误码始终返回 HTTP 200，错误信息在 JSON body 中
        return JsonResponse(unified, isCustomErrorCode ? QHttpServerResponse::StatusCode::Ok : r.statusCode());

    } catch (...) {
<<<<<<< HEAD
        //        Logger::log(Logger::Error, "Response processing failed");
=======
>>>>>>> 346b751 (audit: 代码审计修复 + 新功能)
        return QHttpServerResponse(QHttpServerResponse::StatusCode::InternalServerError);
    }
}

QHttpServerResponse HttpServer::loggingMiddleware(QHttpServerResponse &&r, const QHttpServerRequest &req)
{
    // 日志请求/响应
    qInfo() << QString("Request: %1 %2").arg(methodToString(req.method()), req.url().path());
    qInfo() << QString("Response: %1").arg(static_cast<int>(r.statusCode()));
    return std::move(r);  // 不修改响应
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
    case UserNotFound:      return "用户名或密码不正确";
    case Unauthorized:      return "认证数据不可为空";
    case InvalidParams:     return "无效的参数";
    default:  return QStringLiteral("未知错误");
    }
}

