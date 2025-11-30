#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>

#include "../../internal/server/http_server.h"
#include "../../internal/service/user/user_service.h"
#include "../../api/user/v1/user_route.h"
#include "../../internal/conf/conf.h"
#include "../../internal/data/data.h"
#include "../../internal/pkg/jwt/jwt.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 关键：注册自定义类型！
    qRegisterMetaType<qKratos::JWT::Claims>();

    app.setApplicationName("qKratos");
    app.setApplicationVersion("1.0.0");

    // ==================== Kratos 正宗命令行解析 ====================
    QCommandLineParser parser;
    parser.setApplicationDescription("Qkratos - C++ version of Go Kratos framework");
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
    if (!Config::instance()->load(configPath)) {
        qCritical() << "Failed to load config file:" << configPath;
        return -1;
    }

    if (!DataBaseManager::instance()->connect()) {
        return -1;
    }

    ServerConfig serverConfig = Config::instance()->server();
    HttpServer server(serverConfig.http.timeout);

    server.registerService(new UserServiceImpl, RegisterUserServiceRoutes);
    // 以后加角色、部门等模块只加一行
    // server.registerService(new RoleServiceImpl, RegisterRoleServiceRoutes);

    server.listen(serverConfig.http.addr, serverConfig.http.port);

    return app.exec();
}
