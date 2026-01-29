#ifndef CRONSCHEDULER_H
#define CRONSCHEDULER_H

#include <QObject>

#include <QTimer>
#include <QDateTime>
#include <QUuid>
#include <QDebug>
#include <functional>
#include <memory>
#include <map>
#include <string>

#include "croncpp.h"

class CronJob : public QObject
{
    Q_OBJECT
public:
    using JobFunc = std::function<void()>;

       CronJob(const std::string& cronExpr, JobFunc func, QObject* parent = nullptr)
           : QObject(parent), m_cronExpr(cronExpr), m_func(std::move(func)) {
           try {
               m_cron = cron::make_cron(m_cronExpr);
           } catch (const std::exception& e) {
               qWarning() << "Invalid cron expression:" << QString::fromStdString(m_cronExpr)
                          << ", error:" << e.what();
               throw;
           }
       }

       void execute() {
           if (m_func) m_func();
       }

   private:
       std::string m_cronExpr;
       cron::cronexpr m_cron;
       JobFunc m_func;

       friend class CronScheduler;
};

class CronScheduler : public QObject {
    Q_OBJECT
public:
    explicit CronScheduler(QObject* parent = nullptr) : QObject(parent) {}

    QString addJob(const std::string& cronExpr, CronJob::JobFunc func);
    bool removeJob(const QString& id);
    void clear();

private:
    void scheduleNextRun(const QString& id);

    std::map<QString, std::shared_ptr<CronJob>> m_jobs;
};


#endif // CRONSCHEDULER_H
