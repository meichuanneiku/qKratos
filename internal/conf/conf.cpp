#include "conf.h"

#include <QFile>
//#include <QYamlParser>        // 需要第三方 qyaml，或者直接用 QJson（下面用 QJson 版）
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFileInfo>
#include <QCoreApplication>

Config *Config::m_instance = 0;

Config *Config::instance()
{
    if(m_instance == 0)
        m_instance = new Config();
    return m_instance;
}

bool Config::load(const QString& inputPath)
{
    QString path = inputPath;

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
    auto server = root["server"].toObject();
    auto http = server["http"].toObject();
    m_server.http.addr = http["addr"].toString("0.0.0.0");
    m_server.http.port = http["port"].toInt(8888);
    m_server.http.timeout = http["timeout"].toInt(1000);

    auto ws = server["ws"].toObject();
    m_server.ws.addr = ws["addr"].toString("0.0.0.0");
    m_server.ws.port = ws["port"].toInt(7777);


    qInfo() << "HTTP Server:" << m_server.http.addr << ":" << m_server.http.port;

    auto dataObj = root["data"].toObject();
    auto mysql = dataObj["mysql"].toObject();
    m_data.mysql.host = mysql["host"].toString();
    m_data.mysql.port = mysql["port"].toInt();
    m_data.mysql.user = mysql["user"].toString();
    m_data.mysql.password = mysql["password"].toString();
    m_data.mysql.dbname = mysql["dbname"].toString();

    auto dmsql = dataObj["dmsql"].toObject();
    m_data.dmsql.host = dmsql["host"].toString();
    m_data.dmsql.port = dmsql["port"].toInt();
    m_data.dmsql.user = dmsql["user"].toString();
    m_data.dmsql.password = dmsql["password"].toString();
    m_data.dmsql.dbname = dmsql["dbname"].toString();
    m_data.dmsql.driverType = dmsql["driverType"].toString();

    auto redis = dataObj["redis"].toObject();
    m_data.redis.addr = redis["addr"].toString();
    m_data.redis.password = redis["password"].toString();
    m_data.redis.db = redis["db"].toInt();

    qInfo() << "Config loaded successfully";
    return true;
}
