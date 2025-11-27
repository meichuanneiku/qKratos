#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>

#include "../../internal/server/http_server.h"
#include "../../internal/service/user/user_service.h"
#include "../../api/user/v1/user_route.h"
#include "../../internal/conf/conf.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    app.setApplicationName("qratos");
    app.setApplicationVersion("1.0.0");

    // ==================== Kratos 正宗命令行解析 ====================
    QCommandLineParser parser;
    parser.setApplicationDescription("Qratos - C++ version of Go Kratos framework");
    parser.addHelpOption();
    parser.addVersionOption();

    // 支持短参数 -c 和长参数 --config
    QCommandLineOption configOption(QStringList() << "c" << "config",
                                    "Config file path (yaml/json)",
                                    "file",                           // value name
                                    "config/config.yaml");             // 默认值
    parser.addOption(configOption);
    parser.process(app);

    QString configPath = parser.value(configOption);
    qInfo() << "Loading config from:" << QDir::cleanPath(configPath);

    // ==================== 加载配置 ====================
    if (!Config::instance().load(configPath)) {
        qCritical() << "Failed to load config file:" << configPath;
        return -1;
    }

    HttpServer server;

    server.registerService(new UserServiceImpl, RegisterUserServiceRoutes);
    // 以后加角色、部门等模块只加一行
    // server.registerService(new RoleServiceImpl, RegisterRoleServiceRoutes);

    server.listen(Config::instance().server().port);

    return app.exec();
}
