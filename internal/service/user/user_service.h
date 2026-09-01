#ifndef USERSERVICEIMPL_H
#define USERSERVICEIMPL_H

#include <QHttpServerResponse>
#include <QHttpServerRequest>
#include <QRegularExpressionMatch>
#include "../../biz/user/user_biz.h"

//StatusCode::NoContent = 204：成功但无返回内容。因此可以当作返回查询有问题的标志，可以在中间件中作为判断的依据，执行自定义错误的解析功能
class UserServiceImpl
{
public:
    QHttpServerResponse CreateUser(const QHttpServerRequest& req);
    QHttpServerResponse ListUsers(const QHttpServerRequest& request);
    QHttpServerResponse ListUsers(const int &systemId, const QString &page);
    QHttpServerResponse UpdateUser(const QHttpServerRequest& request);
    QHttpServerResponse UpdateUserById(const int& systemId, quint64 id, const QJsonObject& data);
    QHttpServerResponse GetUser(const int &systemId, const QRegularExpressionMatch& match);
    QHttpServerResponse DeleteUser(const QRegularExpressionMatch& match);
    QHttpServerResponse GetUserByIdDirect(const int& systemId, const QString& id);
    QHttpServerResponse DeleteUser(const int& id);
    QHttpServerResponse GetUserByIdDirect(const QHttpServerRequest &request);


    QHttpServerResponse GetProfile(const QHttpServerRequest &request);

    QHttpServerResponse Login(const int &systemId, const QHttpServerRequest &request);
    QHttpServerResponse Login(const int &systemId, const QString &username, const QString &password);
    QHttpServerResponse UserList(const int &systemId, const QHttpServerRequest &request);
    QHttpServerResponse UserList(const int &systemId, const QString &fxtid, const QString &jsid,
                                const QString &zxzt, const QString &yhmc,
                                const QString &dlcsCondition, const int &dlcs,
                                const int &page, const int &pageSize);

    //测试web推送
    QJsonObject pollAndPush(const QJsonObject &doc);

private:
    UserBiz m_biz;
};

#endif // USERSERVICEIMPL_H
