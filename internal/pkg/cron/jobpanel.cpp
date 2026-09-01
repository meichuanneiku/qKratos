#include "jobpanel.h"

#include <QJsonDocument>
#include <QDebug>
#include <QtConcurrent>
#include <QMetaObject>

#include "../../../internal/server/http_server.h"
#include "../../../internal/biz/user/user_biz.h"

void JobPanel::deviceParam(QString cronExpr)
{
    auto* scheduler = AppContext::instance().scheduler();

    if (!scheduler) return;

    if (!m_sDeviceParamId.isEmpty()) {
        return;
    }

    m_sDeviceParamId = scheduler->addJob(cronExpr.toStdString(), [=]() {
        // 使用 QtConcurrent 在线程池中异步执行 DB 查询和推送
        QtConcurrent::run([=]() {
#if HTTPSERVER_INSTANCE
            auto webSocketMap = HttpServer::instance().getWebSocketMap();
#else
            auto webSocketMap = AppContext::instance().webSocketListB();
#endif
            qDebug()<< "webSocketMap count = " << webSocketMap.count();

            QMapIterator<QWebSocket *, QJsonObject> i(webSocketMap);
            while (i.hasNext()) {
                i.next();

                QJsonObject wsJson = i.value();

                qDebug()<< "i.key() = "<<i.key()<<"         index = " << wsJson["index"];

                UserBiz bizUser;
                JsonObjectResult jResult = bizUser.deviceParamPush(i.value());
                QJsonObject data  = jResult.data;

                QJsonObject response;
                response["type"] = "deviceParam";
                response["data"] = data;
                if(jResult.isSuccess()){
                    response["code"] = 200;
                    response["message"] = "成功";
                }else{
                    response["code"] = jResult.errorCode;
                    response["message"] = "失败";
                }

                wsJson["index"] = 1;
                webSocketMap[i.key()] = wsJson;
                AppContext::instance().setWebSocketB(webSocketMap);

                i.key()->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
            }
        });
    });

}
