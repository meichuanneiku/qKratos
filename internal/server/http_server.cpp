#include "http_server.h"
#include <QDebug>
HttpServer::HttpServer()
{
    m_server.afterRequest([](QHttpServerResponse&& resp, const QHttpServerRequest&) {
            resp.setHeader("X-Powered-By", "Qratos/1.0");
            return std::move(resp);
        });
}

bool HttpServer::listen(quint16 port)
{
    if (m_server.listen(QHostAddress::Any, port)) {
        qDebug() << QString("QHttpServerExample Running on http://127.0.0.1:%1").arg(port);
        return true;
    }
    return false;
}
