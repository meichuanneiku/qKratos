#ifndef USERSERVICEIMPL_H
#define USERSERVICEIMPL_H

#include <QtHttpServer/qhttpserverresponse.h>
#include <QtHttpServer/qhttpserverrequest.h>
#include <QRegularExpressionMatch>
#include "../../biz/user/user_biz.h"

class UserServiceImpl
{
public:
    QHttpServerResponse CreateUser(const QHttpServerRequest& req);
    QHttpServerResponse GetUser(const QRegularExpressionMatch& match);
    QHttpServerResponse DeleteUser(const QRegularExpressionMatch& match);
    QHttpServerResponse GetUserByIdDirect(const int& id);
    QHttpServerResponse DeleteUser(const int& id);
private:
    UserBiz m_biz;
};

#endif // USERSERVICEIMPL_H
