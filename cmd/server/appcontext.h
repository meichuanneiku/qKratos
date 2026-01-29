#ifndef APPCONTEXT_H
#define APPCONTEXT_H

#include "../../internal/pkg/cron/cron_scheduler.h"
#include <QtWebSockets/QWebSocket>
#include <QJsonObject>

class AppContext {
public:
    static AppContext& instance() {
        static AppContext ctx;
        return ctx;
    }

    void setScheduler(CronScheduler* scheduler) { m_scheduler = scheduler; }
    CronScheduler* scheduler() const { return m_scheduler; }


    void setWebSocketA(QList<QWebSocket *> webSocketList) { a = webSocketList; }
    QList<QWebSocket *> webSocketListA() const { return a; }

    void setWebSocketB(QMap<QWebSocket *, QJsonObject> webSocketList) { b = webSocketList; }
    QMap<QWebSocket *, QJsonObject> webSocketListB() const { return b; }

private:
    AppContext() = default;
    CronScheduler* m_scheduler = nullptr;

    QList<QWebSocket *> a;
    QMap<QWebSocket *, QJsonObject> b;
};

#endif // APPCONTEXT_H
