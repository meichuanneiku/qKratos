#include "error_info.h"
/*#include <QHash>

// 方式1：用函数返回（推荐，避免静态初始化顺序问题）
static QHash<int, ErrorInfo> createErrorMap() {
    return QHash<int, ErrorInfo> {
        {1001, {1001, "用户名不能为空"}},
        {1002, {1002, "密码长度不足"}},
        {1003, {1003, "邮箱格式错误"}},
        {1004, {1004, "用户id不能为空"}},
        {1005, {1005, "无效的用户id！"}},
        {2001, {2001, "数据库连接失败"}},
        {2002, {2002, "记录不存在"}},
    };
}

const QHash<int, ErrorInfo> &getErrorRegistry()
{
    static const QHash<int, ErrorInfo> registry = createErrorMap();
    return registry;
}
*/
