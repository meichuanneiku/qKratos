#pragma once

#include <QHttpServerResponse>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "../error/error_code.h"
using namespace  qKratos::Error;

namespace qKratos::response {


inline QHttpServerResponse JsonResponse(const QJsonDocument &doc,
                                QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok)
{
    return QHttpServerResponse("application/json; charset=utf-8",  doc.toJson(QJsonDocument::Compact), status);
}

inline QHttpServerResponse JsonResponse(const QJsonObject &obj,
                                QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok)
{
    return JsonResponse(QJsonDocument(obj), status);
}

inline QHttpServerResponse JsonResponse(const QJsonArray &arr,
                                QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok)
{
    return JsonResponse(QJsonDocument(arr), status);
}

inline QHttpServerResponse TextResponse(const QString &text,
                                QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::Ok)
{
    return QHttpServerResponse("text/plain; charset=utf-8",  text.toUtf8(), status);
}

/*inline QHttpServerResponse ErrorResponse(const QString &text,
                                QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::NoContent)
{
    return TextResponse(text, status);
}

inline QHttpServerResponse DeleteResponse(
                                QHttpServerResponse::StatusCode status = QHttpServerResponse::StatusCode::NoContent)
{
    return JsonResponse(SuccessBody(), status);
}*/

inline QHttpServerResponse Status(ErrorCode code)
{
    return QHttpServerResponse(static_cast<QHttpServerResponse::StatusCode>(code));
}

inline QHttpServerResponse Status(QHttpServerResponse::StatusCode code = QHttpServerResponse::StatusCode::Ok)
{
    return QHttpServerResponse(code);
}

} // namespace qratos::response
