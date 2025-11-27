#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QtHttpServer/qhttpserver.h>

class HttpServer
{
public:
    HttpServer();

    template<typename Impl, typename RegisterFunc>
    void registerService(Impl* impl, RegisterFunc registerFunc)
    {
        registerFunc(m_server, impl);
    }

        bool listen(quint16 port = 8080);
private:
    QHttpServer m_server;
};

#endif // HTTPSERVER_H
