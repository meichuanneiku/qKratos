#include "redis.h"
#include <QDebug>

namespace qKratos::Cache {

RedisCache::RedisCache(const QString& host, quint16 port,
                       const QString& password, int db)
    : m_host(host), m_port(port), m_password(password), m_db(db), m_connected(false)
{
    m_socket = new QTcpSocket();
}

RedisCache::~RedisCache()
{
    disconnect();
    delete m_socket;
}

bool RedisCache::connect()
{
    if (m_connected) return true;

    m_socket->connectToHost(m_host, m_port);
    if (!m_socket->waitForConnected(3000)) {
        qWarning() << "Redis connection failed:" << m_socket->errorString();
        return false;
    }

    if (!m_password.isEmpty()) {
        QByteArray authResp = sendCommand(buildCommand("AUTH", {m_password.toUtf8()}))
                ? readReply() : QByteArray();
        if (authResp.isEmpty() || authResp[0] == '-') {
            qWarning() << "Redis AUTH failed";
            m_socket->disconnectFromHost();
            return false;
        }
    }

    if (m_db > 0) {
        QByteArray selectResp = sendCommand(buildCommand("SELECT", {QByteArray::number(m_db)}))
                ? readReply() : QByteArray();
        if (selectResp.isEmpty() || selectResp[0] == '-') {
            qWarning() << "Redis SELECT failed";
        }
    }

    m_connected = true;
    qInfo() << "Redis connected to" << m_host << ":" << m_port;
    return true;
}

void RedisCache::disconnect()
{
    if (m_connected) {
        m_socket->disconnectFromHost();
        m_connected = false;
    }
}

bool RedisCache::isConnected() const
{
    return m_connected && m_socket->state() == QAbstractSocket::ConnectedState;
}

QByteArray RedisCache::buildCommand(const QByteArray& cmd, const QList<QByteArray>& args)
{
    QByteArray result;
    int totalArgs = 1 + args.size();
    result.append('*').append(QByteArray::number(totalArgs)).append("\r\n");
    result.append('$').append(QByteArray::number(cmd.size())).append("\r\n");
    result.append(cmd).append("\r\n");
    for (const auto& arg : args) {
        result.append('$').append(QByteArray::number(arg.size())).append("\r\n");
        result.append(arg).append("\r\n");
    }
    return result;
}

bool RedisCache::sendCommand(const QByteArray& data)
{
    if (!isConnected()) return false;
    m_socket->write(data);
    return m_socket->waitForBytesWritten(3000);
}

QByteArray RedisCache::readReply()
{
    if (!m_socket->waitForReadyRead(5000)) return QByteArray();

    QByteArray line = m_socket->readLine();
    if (line.isEmpty()) return QByteArray();

    char type = line[0];
    QByteArray content = line.mid(1).trimmed();

    switch (type) {
    case '+': // 简单字符串
        return content;
    case '-': // 错误
        qWarning() << "Redis error:" << content;
        return QByteArray(1, '-') + content;
    case ':': // 整数
        return content;
    case '$': { // 批量字符串
        int len = content.toInt();
        if (len == -1) return QByteArray(); // nil
        QByteArray bulk = m_socket->read(len + 2); // + \r\n
        return bulk.left(len);
    }
    case '*': { // 数组
        int count = content.toInt();
        QByteArray result;
        for (int i = 0; i < count; ++i) {
            QByteArray element = readReply();
            if (i > 0) result.append('\n');
            result.append(element);
        }
        return result;
    }
    default:
        return line;
    }
}

bool RedisCache::set(const QString& key, const QByteArray& value, int ttlSeconds)
{
    if (!sendCommand(buildCommand("SET", {key.toUtf8(), value}))) return false;
    QByteArray reply = readReply();
    if (reply.isEmpty() || reply[0] == '-') return false;

    if (ttlSeconds > 0) {
        sendCommand(buildCommand("EXPIRE", {key.toUtf8(), QByteArray::number(ttlSeconds)}));
        readReply();
    }
    return true;
}

QByteArray RedisCache::get(const QString& key)
{
    if (!sendCommand(buildCommand("GET", {key.toUtf8()}))) return QByteArray();
    return readReply();
}

bool RedisCache::del(const QString& key)
{
    if (!sendCommand(buildCommand("DEL", {key.toUtf8()}))) return false;
    QByteArray reply = readReply();
    return !reply.isEmpty() && reply[0] != '-';
}

bool RedisCache::exists(const QString& key)
{
    if (!sendCommand(buildCommand("EXISTS", {key.toUtf8()}))) return false;
    QByteArray reply = readReply();
    return !reply.isEmpty() && reply[0] != '-' && reply.toInt() > 0;
}

} // namespace qKratos::Cache
