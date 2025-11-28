#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QtHttpServer/qhttpserver.h>

#include <QJsonParseError>
#include <QJsonObject>

class HttpServer
{
public:
    HttpServer(const int &timeout = 2000);

    template<typename Impl, typename RegisterFunc>
    void registerService(Impl* impl, RegisterFunc registerFunc)
    {
        registerFunc(m_server, impl);
    }

    bool listen(const QString &address = "0.0.0.0", const quint16 &port = 8080);
private:

    static QHttpServerResponse responseMiddleware(QHttpServerResponse &&r, const QHttpServerRequest &req);
    static QString getMessage(int code, const QString &defaultMsg);

    QHttpServer m_server;
};

#endif // HTTPSERVER_H
