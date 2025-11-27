#include <QCoreApplication>

#include "../../internal/server/http_server.h"
#include "../../internal/service/user/user_service.h"
#include "../../api/user/v1/user_route.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    HttpServer server;

        server.registerService(new UserServiceImpl, RegisterUserServiceRoutes);
        // 以后加角色、部门等模块只加一行
        // server.registerService(new RoleServiceImpl, RegisterRoleServiceRoutes);

        server.listen(8080);

    return a.exec();
}
