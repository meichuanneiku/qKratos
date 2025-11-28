QT -= gui

QT += core network httpserver sql

TARGET = qratos-server

CONFIG += c++17 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        api/user/v1/user_route.cpp \
        cmd/server/main.cpp \
        internal/biz/user/user_biz.cpp \
        internal/conf/conf.cpp \
        internal/data/data.cpp \
        internal/data/user/user_repo.cpp \
        internal/pkg/errorinfo/error_info.cpp \
        internal/server/http_server.cpp \
        internal/service/user/user_service.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    api/user/v1/user_route.h \
    internal/biz/user/user_biz.h \
    internal/conf/conf.h \
    internal/data/data.h \
    internal/data/user/user_repo.h \
    internal/pkg/errorinfo/error_info.h \
    internal/server/http_server.h \
    internal/service/user/user_service.h

DISTFILES += \
    configs/config.json
