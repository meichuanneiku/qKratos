#include "jobpanel.h"

#include <QJsonDocument>
#include <QDebug>
#include <QtConcurrent>
#include <QMetaObject>

#include "../../../internal/server/http_server.h"
#include "../../../internal/data/user/user_repo.h"

void JobPanel::deviceParam(QString cronExpr)
{
    auto* scheduler = AppContext::instance().scheduler();

    if (!scheduler) return;

    if (!m_sDeviceParamId.isEmpty()) {
        return;
    }

    m_sDeviceParamId = scheduler->addJob(cronExpr.toStdString(), [=]() {
//        QtConcurrent::run([=]() {
        QTimer::singleShot(0, this, [=]() {
#if HTTPSERVER_INSTANCE
            auto webSocketMap = HttpServer::instance().getWebSocketMap();
#else
            auto webSocketMap = AppContext::instance().webSocketListB();
#endif
//            qDebug()<< "webSocketMap count = " << webSocketMap.count();

            QMapIterator<QWebSocket *, QJsonObject> i(webSocketMap);
            while (i.hasNext()) {
                i.next();

                QJsonObject wsJson = i.value();

                qDebug()<< "i.key() = "<<i.key()<<"         index = " << wsJson["index"];
                QJsonObject data = UserRepo::instance().deviceParamPush(i.value());

                wsJson["index"] = 1;
                webSocketMap[i.key()] = wsJson;
                AppContext::instance().setWebSocketB(webSocketMap);

                QJsonObject response;
                response["type"] = "deviceParam";
                response["code"] = 200;
                response["message"] = "成功";
                response["data"] = data;
                i.key()->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
            }
        });
    });

}
