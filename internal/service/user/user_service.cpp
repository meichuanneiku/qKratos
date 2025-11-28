#include "user_service.h"

#include <QJsonDocument>
#include <QJsonObject>

QHttpServerResponse UserServiceImpl::CreateUser(const QHttpServerRequest& req)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(req.body(), &err);
    if (err.error != QJsonParseError::NoError){

//        QHttpServerResponse resp("name required");
//        resp.setStatusCode(QHttpServerResponse::StatusCode::BadRequest);
//        return resp;

        return QHttpServerResponse("text/plain; charset=utf-8","invalid json", QHttpServerResponse::StatusCode::BadRequest);
    }

    QString name = doc.object()["name"].toString();
    if (name.isEmpty())
        return QHttpServerResponse("text/plain; charset=utf-8","name required", QHttpServerResponse::StatusCode::BadRequest);

    int id = m_biz.CreateUser(name);

    QJsonObject body{ {"id", id}, {"name", name} };
    return QHttpServerResponse("application/json; charset=utf-8", QJsonDocument(body).toJson(QJsonDocument::Compact),
                               QHttpServerResponse::StatusCode::Created);
}

QHttpServerResponse UserServiceImpl::GetUser(const int &systemId, const QRegularExpressionMatch& match)
{
    bool ok;
    int id = match.captured(1).toInt(&ok);
    if (!ok) return QHttpServerResponse("text/plain; charset=utf-8","invalid id", QHttpServerResponse::StatusCode::BadRequest);

    auto user = m_biz.GetUser(systemId, QString::number(id));
    if (user.isEmpty()) return QHttpServerResponse("text/plain; charset=utf-8","not found", QHttpServerResponse::StatusCode::NotFound);

    return QHttpServerResponse(QJsonDocument(user).toJson());
}

QHttpServerResponse UserServiceImpl::DeleteUser(const QRegularExpressionMatch& match)
{
    bool ok;
    int id = match.captured(1).toInt(&ok);
    if (!ok) return QHttpServerResponse("text/plain; charset=utf-8","invalid id", QHttpServerResponse::StatusCode::NoContent);

    m_biz.DeleteUser(id);
    return QHttpServerResponse("deleted");
}

QHttpServerResponse UserServiceImpl::GetUserByIdDirect(const int &systemId, const QString &id)
{
    auto user = m_biz.FindById(systemId, id);
    if (user.isEmpty())
        return QHttpServerResponse("text/plain; charset=utf-8", "1005", QHttpServerResponse::StatusCode::NoContent);

    return QHttpServerResponse(user);
}

QHttpServerResponse UserServiceImpl::DeleteUser(const int &id)
{
    m_biz.DeleteUser(id);
    return QHttpServerResponse("deleted");
}
