#include "runanalyzeserviceimpl.h"

#include "../../pkg/middleware/auth.h"
using namespace qKratos::Middleware;

QHttpServerResponse RunAnalyzeServiceImpl::pollAndPush(QJsonObject doc)
{
    if (!doc.isEmpty()) {
        return Status(InvalidParams);  // 自定义错误码：参数错误
    }
}
