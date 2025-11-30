// pkg/errorcode/error_code.h
#pragma once
#include <QHttpServerResponse>
#include <QJsonObject>
#include <QJsonDocument>

namespace qKratos::Error {

enum ErrorCode : int {
    Success          = 0,
    InvalidParams = 1000,
    UserNameEmpty    = 1001,
    PasswordTooShort =1002,
    EmailInvalid     =1003,
    UserIdEmpty      =1004,
   UserIdInvalid     =1006,
    UserNotFound     =1005,
    Unauthorized = 1007,
    UnknownError     =9999
};

constexpr const char* message(ErrorCode code) noexcept {
    switch (code) {
    case Success:           return "成功";
    case UserNameEmpty:     return "用户名不能为空";
    case UserNotFound:      return "用户不存在";
        // ... 其他
    default:                return "未知错误";
    }
}

// 关键：返回一个“特殊状态码”的响应，中间件能识别！
inline QHttpServerResponse ErrorResponse(ErrorCode code)
{
    if (code == Success) {
        return QHttpServerResponse(QHttpServerResponse::StatusCode::Ok); // 不会走到这里
    }

    // 使用 600 系列状态码作为“标记”，表示这是我们自定义错误
    // （600 + code） 绝对不会和 HTTP 标准状态码冲突！
    QJsonObject body{
        {"code", static_cast<int>(code)},
        {"message", message(code)},
        {"data", QJsonObject{}}
    };

        return QHttpServerResponse(
            QJsonDocument(body).toJson(QJsonDocument::Compact)
        );

//    QByteArray jsonData = QJsonDocument(body).toJson(QJsonDocument::Compact);

//    // 正确写法：必须用 (mimeType, data, status) 三个参数的构造函数
//    return QHttpServerResponse(
//                "application/json; charset=utf-8",           // mimeType
//                jsonData,                                    // data
//                static_cast<QHttpServerResponse::StatusCode>(600 + code)  // 标记状态码
//                );
}

inline QHttpServerResponse SuccessResponse(const QJsonObject& data = {})
{
    QJsonObject body{
        {"code", 200},
        {"message", "成功"},
        {"data", data}
    };
    return QHttpServerResponse(
        QJsonDocument(body).toJson(QJsonDocument::Compact)
    );
//    return QHttpServerResponse(
//                "application/json; charset=utf-8",
//                QJsonDocument(body).toJson(QJsonDocument::Compact),
//                QHttpServerResponse::StatusCode::Ok
//                );
}

} // namespace
