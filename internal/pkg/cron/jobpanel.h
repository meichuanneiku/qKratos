#ifndef JOBPANEL_H
#define JOBPANEL_H

#include <QObject>
#include "../../../cmd/server/appcontext.h"

class JobPanel : public QObject
{
    Q_OBJECT
public:

    static JobPanel& instance()
    {
        static JobPanel jobPanel;   // C++11 起支持的线程安全单例
        return jobPanel;
    }

    //参数追溯分析
    void deviceParam(QString cronExpr);

signals:

private:
    JobPanel() = default;

    QString m_sDeviceParamId;

};

#endif // JOBPANEL_H
