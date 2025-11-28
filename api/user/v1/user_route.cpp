#include "user_route.h"

#include "../../../internal/service/user/user_service.h"

static inline QString host(const QHttpServerRequest &request)
{
    return request.headers()[QStringLiteral("Host")].toString();
}

void RegisterUserServiceRoutes(QHttpServer& server, UserServiceImpl* impl)
{
    server.route("/api/v1/users", QHttpServerRequest::Method::Post,
                 [impl](const QHttpServerRequest& req) {
                     return impl->CreateUser(req);
                 });



//    server.route("/api/v1/users/<arg>",
//                 [impl](const QRegularExpressionMatch& match) {
//                     return impl->GetUser(match);
//                 });

//    server.route("/api/v1/users/<arg>", QHttpServerRequest::Method::Delete,
//                 [impl](const QRegularExpressionMatch& match) {
//                     return impl->DeleteUser(match);
//                 });



    // GET /api/v1/users/123   ← 手动解析 ID
    // 2. 关键修改：用正则匹配任意长度的用户ID路径（支持 123、123/、abc123 等）
//        server.route<QHttpServerRouterRule>(R"(/api/v1/users/(.+))",
//                     [impl](const QRegularExpressionMatch& match, const QHttpServerRequest& req) -> QHttpServerResponse {
//            QString idStr = match.captured(1);           // 捕获 () 里的内容
//            if (idStr.endsWith('/')) idStr.chop(1);      // 去掉可能的末尾斜杠

//            bool ok;
//            int id = idStr.toInt(&ok);
////            if (!ok || id <= 0)
////                return qratos::response::Text("invalid user id", 400);

//            if (req.method() == QHttpServerRequest::Method::Get)
//                return impl->GetUserByIdDirect(id);

//            if (req.method() == QHttpServerRequest::Method::Delete) {
//                return impl->DeleteUser(id);
////                return qratos::response::Text("deleted");
//            }

////            return qratos::response::Status(QHttpServerResponse::StatusCode::MethodNotAllowed);
//            return QHttpServerResponse(QHttpServerResponse::StatusCode::MethodNotAllowed);
//        });

//    // DELETE 同样处理
//    server.route("/api/v1/users/", QHttpServerRequest::Method::Delete,
//                 [impl](const QHttpServerRequest& req) {
//        // 同上解析 id 解析逻辑...
//        // return impl->DeleteUser(id);
//        QString path = req.url().path();                    // "/api/v1/users/123"
//        QStringList parts = path.split('/', QString::SkipEmptyParts);

//        bool ok;
//        int id = parts[3].toInt(&ok);
//        return impl->DeleteUser(id);  // 改个函数名，避免 match 参数
//    });



    server.route("/api/v1/users/<arg>",QHttpServerRequest::Method::Get, [&] (quint64 id, const QHttpServerRequest &request) {

        QByteArray systemId = request.headers().value("systemid").toByteArray();
        return impl->GetUserByIdDirect(systemId.toInt(), QString::number(id));
      });

//    server.route("/api/v1/users",QHttpServerRequest::Method::Post, [impl] (const QHttpServerRequest &req) {
//        return impl->GetUserList();
//      });
}
