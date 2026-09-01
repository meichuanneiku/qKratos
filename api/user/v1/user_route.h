#pragma once

#include <QHttpServer>
#include <QHttpServerRequest>

class UserServiceImpl;   // forward declaration

void RegisterUserServiceRoutes(QHttpServer& server, UserServiceImpl* impl);
