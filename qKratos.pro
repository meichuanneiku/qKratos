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

 #jwt-cpp
INCLUDEPATH += $$PWD/third_party/jwt-cpp/include

win32 {
# 1. 强制向系统询问 g++ 的绝对路径
# 使用 Windows 的 "where" 命令查找 g++
FULL_CXX_RAW = $$system("where g++")

# 2. 处理路径格式 (非常重要！)
# Windows 返回的路径可能是多行(如果有多个g++)，且使用反斜杠 \
# 我们取第一行，并将 \ 替换为 / 以便 QMake 处理
FULL_CXX_PATH = $$member(FULL_CXX_RAW, 0)
FULL_CXX_PATH = $$replace(FULL_CXX_PATH, \\\\, /)

# 4. 开始推导目录
# 从 .../bin/g++.exe 去掉文件名 -> .../bin
COMPILER_BIN = $$dirname(FULL_CXX_PATH)

# 从 .../bin 去掉 bin -> .../ (工具链根目录)
TOOLCHAIN_ROOT = $$dirname(COMPILER_BIN)

# 5. 拼接 OpenSSL 路径
# 假设结构是 C:/Qt/Tools/mingw730_64/opt/lib
OPENSSL_LIB_PATH = $$TOOLCHAIN_ROOT/opt/lib
OPENSSL_INC_PATH = $$TOOLCHAIN_ROOT/opt/include

# 4. 应用配置
INCLUDEPATH += $$OPENSSL_INC_PATH
LIBS += -L$$OPENSSL_LIB_PATH -lssl -lcrypto -lws2_32 -lcrypt32
}


INCLUDEPATH += $$PWD/third_party/casbin-cpp/include
LIBS += -L$$PWD/third_party/casbin-cpp/bin -lcasbin
