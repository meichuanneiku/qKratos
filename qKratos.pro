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
        internal/pkg/casbin/casbin_dm_adapter.cpp \
        internal/pkg/jwt/jwt.cpp \
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
    internal/pkg/casbin/casbin_dm_adapter.h \
    internal/pkg/error/error_code.h \
    internal/pkg/jwt/jwt.h \
    internal/pkg/middleware/auth.h \
    internal/pkg/response/response.h \
    internal/server/http_server.h \
    internal/service/user/user_service.h

DISTFILES += \
    configs/config.json

#INCLUDEPATH += $$PWD/third_party/casbin-cpp/include
#SOURCES += \
#    third_party/casbin-cpp/casbin/enforcer.cpp \
#    third_party/casbin-cpp/casbin/enforcer_cached.cpp \
#    third_party/casbin-cpp/casbin/enforcer_synced.cpp \
#    third_party/casbin-cpp/casbin/model/model.cpp \
#    third_party/casbin-cpp/casbin/model/assertion.cpp \
#    third_party/casbin-cpp/casbin/model/evaluator.cpp \
#    third_party/casbin-cpp/casbin/model/function.cpp \
##    third_party/casbin-cpp/casbin/util.cpp \
##    third_party/casbin-cpp/casbin/built_in_functions.cpp \
##    third_party/casbin-cpp/casbin/effect/effect.cpp \
#    third_party/casbin-cpp/casbin/effect/default_effector.cpp \
#    third_party/casbin-cpp/casbin/rbac/default_role_manager.cpp \
##    third_party/casbin-cpp/casbin/rbac/rbac.cpp \
#    third_party/casbin-cpp/casbin/persist/file_adapter/file_adapter.cpp \
##    third_party/casbin-cpp/casbin/persist/filtered_adapter.cpp \
##    third_party/casbin-cpp/casbin/persist/persist.cpp \
#    third_party/casbin-cpp/casbin/logger.cpp \
#    third_party/casbin-cpp/casbin/config/config.cpp



INCLUDEPATH += $$PWD/third_party/casbin-cpp/include
LIBS += -L$$PWD/third_party/casbin-cpp/bin -lcasbin
