#pragma once
#include <QString>

struct ErrorInfo {
    int code = 0;
    QString message;

    ErrorInfo() = default;
    ErrorInfo(int c, const QString& msg) : code(c), message(msg) {}
};




// 全局访问点（线程安全，C++11 起 static 局部变量初始化是线程安全的）
const QHash<int, ErrorInfo>& getErrorRegistry();
