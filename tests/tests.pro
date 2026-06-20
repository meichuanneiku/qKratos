QT += testlib core network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

TARGET = tst_qKratos

INCLUDEPATH += $$PWD/..

SOURCES += \
    tst_qKratos.cpp

INCLUDEPATH += $$PWD/../third_party/jwt-cpp/include
