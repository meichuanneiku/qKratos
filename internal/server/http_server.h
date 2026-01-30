#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QHttpServer>
#include <QJsonParseError>
#include <QJsonObject>
#include <QDebug>
#include <QtWebSockets/QWebSocket>

class HttpServer : public QObject
{
    Q_OBJECT
public:

#if HTTPSERVER_INSTANCE
      static HttpServer& instance() {
        static HttpServer server;
        return server;
    }

    void init(const int &timeout = 2000);

    void initWebSocket();
#else
    HttpServer(const int &timeout = 2000, QObject *parent= nullptr);
#endif



    ~HttpServer();

    template<typename Impl, typename RegisterFunc>
    void registerService(Impl* impl, RegisterFunc registerFunc)
    {
        registerFunc(m_server, impl);
    }

    bool listen(const QString &address = "0.0.0.0", const quint16 &port = 8080);


    QMap<QWebSocket *, QJsonObject> getWebSocketMap(){return  m_webClientB; }
public slots:
    // 处理新的 WebSocket 连接的槽函数
    void onNewConnection();

private:
#if HTTPSERVER_INSTANCE
    HttpServer() = default;
#endif
    static QHttpServerResponse responseMiddleware(QHttpServerResponse &&r, const QHttpServerRequest &req);
    static QHttpServerResponse loggingMiddleware(QHttpServerResponse &&r, const QHttpServerRequest &req);
    static QString getMessage(int status);

    QHttpServer m_server;

    // 存储所有连接的客户端的列表
    QList<QWebSocket *> m_webClients;
    QMap<QWebSocket *, QJsonObject> m_webClientB;
};

#endif // HTTPSERVER_H
