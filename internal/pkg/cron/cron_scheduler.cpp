#include "cron_scheduler.h"

QString CronScheduler::addJob(const std::string& cronExpr, CronJob::JobFunc func) {
    auto job = std::make_shared<CronJob>(cronExpr, std::move(func), this);
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_jobs[id] = job;
    scheduleNextRun(id);
    return id;
}

bool CronScheduler::removeJob(const QString& id) {
    return m_jobs.erase(id) > 0;
}

void CronScheduler::clear() {
    m_jobs.clear();
}

void CronScheduler::scheduleNextRun(const QString& id) {
    auto it = m_jobs.find(id);
    if (it == m_jobs.end()) return;

    auto now = std::chrono::system_clock::now();
    auto next = cron::cron_next(it->second->m_cron, now);

    // 计算精确的毫秒差
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(next - now).count();

    if (duration_ms <= 0) {
        // 立即计算下一次执行时间
//        scheduleNextRun(id);
        QTimer::singleShot(100,  Qt::PreciseTimer, this, [this, id]() {
                       scheduleNextRun(id);
                   });
        return;
    }

    //使用高精度计算结果
    qint64 msecs = static_cast<qint64>(duration_ms);

#if 0
    // 调试信息：显示真实的本地时间（带毫秒）
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
    auto next_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                next.time_since_epoch()).count();

    QDateTime nowQt = QDateTime::fromMSecsSinceEpoch(now_ms);
    QDateTime nextQt = QDateTime::fromMSecsSinceEpoch(next_ms);

    qDebug() << "Job" << id << "scheduled:"
             << "Now:" << nowQt.toString("yyyy-MM-dd hh:mm:ss.zzz")
             << "Next:" << nextQt.toString("yyyy-MM-dd hh:mm:ss.zzz")
             << "Delay:" << msecs << "ms";

#endif
    //Qt::PreciseTimer 精密计时器力求达到毫秒级的精度
    QTimer::singleShot(msecs, Qt::PreciseTimer, this, [this, id]() {
        auto it2 = m_jobs.find(id);
        if (it2 != m_jobs.end()) {
            // 执行 Job
            it2->second->execute();
            scheduleNextRun(id);
        }
    });
}
