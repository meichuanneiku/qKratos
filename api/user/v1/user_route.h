#pragma once

#include <QHttpServer>
#include <QHttpServerRequest>

class UserServiceImpl;   // 前向声明

void RegisterUserServiceRoutes(QHttpServer& server, UserServiceImpl* impl);

