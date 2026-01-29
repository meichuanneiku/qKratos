#ifndef RUNANALYZESERVICEIMPL_H
#define RUNANALYZESERVICEIMPL_H

#include <QHttpServerResponse>
#include <QHttpServerRequest>
#include <QJsonObject>


//#include "../../biz/run_analyze/run_analyze_biz.h"

class RunAnalyzeServiceImpl
{

    QHttpServerResponse pollAndPush(QJsonObject doc);
};

#endif // RUNANALYZESERVICEIMPL_H
