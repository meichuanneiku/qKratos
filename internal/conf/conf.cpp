#include "conf.h"

#include <QFile>
//#include <QYamlParser>        // 需要第三方 qyaml，或者直接用 QJson（下面用 QJson 版）
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>

bool Config::load(const QString& inputPath)
{
    /*QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Config file not found, use default:" << path;
        return false;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qCritical() << "Parse config failed:" << err.errorString();
        return false;
    }

    auto root = doc.object();
    auto serverObj = root["server"].toObject()["http"].toObject();

    server_.addr = serverObj["addr"].toString("0.0.0.0");
    server_.port = serverObj["port"].toInt(8080);
    server_.timeoutSec = serverObj["timeout"].toString("30s").remove('s').toInt();

    qInfo() << "Config loaded: HTTP listening on" << server_.addr << ":" << server_.port;*/

    QString path = inputPath;
    if (path.isEmpty()) path = "config/config.yaml";

    QFileInfo info(path);
    if (!info.exists()) {
        // 尝试相对程序目录的常见位置（Kratos 风格）
        QStringList tryPaths = {
            QCoreApplication::applicationDirPath() + "/../" + path,
            QCoreApplication::applicationDirPath() + "/../../" + path,
            QCoreApplication::applicationDirPath() + "/" + path,
            path
        };
        for (const auto& p : tryPaths) {
            if (QFile::exists(p)) {
                path = p;
                break;
            }
        }
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file:" << path;
        return false;
    }

    QJsonParseError jsonErr;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError) {
        qCritical() << "Config parse error:" << jsonErr.errorString() << "at offset" << jsonErr.offset;
        return false;
    }

    auto root = doc.object();
    auto http = root["server"].toObject()["http"].toObject();

    server_.addr = http["addr"].toString("0.0.0.0");
    server_.port = http["port"].toInt(8080);

    qInfo() << "Config loaded successfully";
    qInfo() << "HTTP Server:" << server_.addr << ":" << server_.port;

    return true;
}
