#include "logger.h"
#include "../../conf/conf.h"

#include <iostream>

using namespace std;

// 静态初始化
//QDebugLogger* QDebugLogger::m_instance = nullptr;
QMutex QDebugLogger::m_mutex;

/*QDebugLogger* QDebugLogger::instance()
{
    if (!m_instance)
        m_instance = new QDebugLogger();
    return m_instance;
}

QDebugLogger::QDebugLogger() : QObject(parent),
    m_logFile(nullptr),
    m_textStream(nullptr),
    m_logDir("logs"),
    m_filePrefix(""),
    m_maxSize(10 * 1024 * 1024),
    m_logLevel(QtDebugMsg),
    m_consoleOutput(false),
    m_async(false),
    m_loggerThread(nullptr)
{
    // 注册消息处理函数
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        QDebugLogger::instance()->handleMessage(type, context, msg);
    });

    // 新增：在应用启动时注册元类型
    qRegisterMetaType<LogContext>("LogContext");
    qRegisterMetaType<LogMessage>("LogMessage");
}
*/
QDebugLogger::~QDebugLogger()
{
    if (m_loggerThread) {
        m_loggerThread->stop();
        if (m_loggerThread->isRunning()) {
            m_loggerThread->wait(5000); // 等待最多5秒
        }
        delete m_loggerThread;
    }

    if (m_textStream)
        delete m_textStream;

    if (m_logFile) {
        if (m_logFile->isOpen())
            m_logFile->close();
        delete m_logFile;
    }
}


void QDebugLogger::init()
{
     Config* pConf = Config::instance();
     if (!pConf) return;

    // 注册消息处理函数
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        QDebugLogger::instance().handleMessage(type, context, msg);
    });

    // 新增：在应用启动时注册元类型
    qRegisterMetaType<LogContext>("LogContext");
    qRegisterMetaType<LogMessage>("LogMessage");


    QMutexLocker locker(&m_mutex);

    m_logDir = pConf->log().logDir;
    m_filePrefix = pConf->log().prefix;
    m_maxSize = pConf->log().maxSize;
    // Convert log level string to QtMsgType
    QString logLevelStr = pConf->log().logLevel;
    if (logLevelStr.contains("Debug", Qt::CaseInsensitive)) m_logLevel = QtDebugMsg;
    else if (logLevelStr.contains("Info", Qt::CaseInsensitive)) m_logLevel = QtInfoMsg;
    else if (logLevelStr.contains("Warning", Qt::CaseInsensitive)) m_logLevel = QtWarningMsg;
    else if (logLevelStr.contains("Critical", Qt::CaseInsensitive)) m_logLevel = QtCriticalMsg;
    else m_logLevel = QtDebugMsg;
    m_consoleOutput = pConf->log().consoleOutput;

    // 如果异步模式改变，需要重新设置线程
    bool asyncEnabled = pConf->log().async;
    if (m_async != asyncEnabled) {
        if (m_loggerThread) {
            m_loggerThread->stop();
            m_loggerThread->wait();
            delete m_loggerThread;
            m_loggerThread = nullptr;
        }

        m_async = asyncEnabled;

        if (m_async) {
            m_loggerThread = new LoggerThread(this);
            // 使用Qt 5兼容的信号槽连接方式
            connect(m_loggerThread, &LoggerThread::logMessageReady,
                    this, &QDebugLogger::processLogMessage);
            m_loggerThread->start();
        }
    }

    ensureLogDirectory();
    checkAndCreateLogFile();
}

void QDebugLogger::setLogLevel(QtMsgType level)
{
    QMutexLocker locker(&m_mutex);
    m_logLevel = level;
}

void QDebugLogger::setMaxFileSize(qint64 size)
{
    QMutexLocker locker(&m_mutex);
    m_maxSize = size;
}

void QDebugLogger::setLogDirectory(const QString& dir)
{
    QMutexLocker locker(&m_mutex);

    if (m_logDir == dir)
        return;

    m_logDir = dir;

    // 如果目录改变，关闭当前文件并创建新文件
    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;

        if (m_textStream) {
            delete m_textStream;
            m_textStream = nullptr;
        }
    }

    ensureLogDirectory();
    checkAndCreateLogFile();
}

void QDebugLogger::setFileNamePrefix(const QString& prefix)
{
    QMutexLocker locker(&m_mutex);

    if (m_filePrefix == prefix)
        return;

    m_filePrefix = prefix;

    // 如果前缀改变，关闭当前文件并创建新文件
    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;

        if (m_textStream) {
            delete m_textStream;
            m_textStream = nullptr;
        }
    }

    checkAndCreateLogFile();
}

void QDebugLogger::setConsoleOutput(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_consoleOutput = enabled;
}

void QDebugLogger::setAsyncMode(bool async)
{
    QMutexLocker locker(&m_mutex);

    if (m_async == async)
        return;

    // 如果异步模式改变，需要重新设置线程
    if (m_loggerThread) {
        m_loggerThread->stop();
        m_loggerThread->wait();
        delete m_loggerThread;
        m_loggerThread = nullptr;
    }

    m_async = async;

    if (m_async) {
        m_loggerThread = new LoggerThread(this);
        // 关键修改：确保信号和槽参数类型完全匹配
        connect(m_loggerThread, SIGNAL(logMessageReady(const LogMessage&)),
                this, SLOT(processLogMessage(const LogMessage&)));
        m_loggerThread->start();
    }
}

bool QDebugLogger::ensureLogDirectory()
{
    QDir dir(m_logDir);
    if (!dir.exists())
        return dir.mkpath(".");
    return true;
}

bool QDebugLogger::checkAndCreateLogFile()
{
    QString newFileName = getCurrentLogFileName();

    // 如果文件名改变或者文件不存在，创建新文件
    if (newFileName != m_currentFileName ||
        (m_logFile && !m_logFile->exists())) {

        // 关闭当前文件
        if (m_logFile && m_logFile->isOpen()) {
            m_logFile->close();
            delete m_logFile;
            m_logFile = nullptr;

            if (m_textStream) {
                delete m_textStream;
                m_textStream = nullptr;
            }
        }

        // 创建新文件
        m_currentFileName = newFileName;
        m_logFile = new QFile(m_currentFileName);

        if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
            m_textStream = new QTextStream(m_logFile);
            m_textStream->setCodec("UTF-8");
            return true;
        } else {
            delete m_logFile;
            m_logFile = nullptr;
            cout << "Failed to open log file:" << m_currentFileName.toStdString();
            return false;
        }
    }

    // 检查文件大小
    if (m_logFile && m_logFile->size() >= m_maxSize) {
        // 文件已达到最大大小，关闭当前文件
        m_logFile->close();

        // 添加时间戳重命名现有文件
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString backupFileName = m_currentFileName + "." + timestamp;

        if (m_logFile->rename(backupFileName)) {
            cout << "Log file reached maximum size. Renamed to:" << backupFileName.toStdString();

            // 创建新的日志文件
            delete m_logFile;
            m_logFile = new QFile(m_currentFileName);

            if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
                m_textStream = new QTextStream(m_logFile);
                m_textStream->setCodec("UTF-8");
                return true;
            } else {
                delete m_logFile;
                m_logFile = nullptr;
                cout << "Failed to create new log file:" << m_currentFileName.toStdString();
                return false;
            }
        } else {
            // 继续使用现有文件，即使它已经超过了最大大小
            if (!m_logFile->isOpen() && m_logFile->open(QIODevice::Append | QIODevice::Text)) {
                m_textStream = new QTextStream(m_logFile);
                m_textStream->setCodec("UTF-8");
                return true;
            }
            return false;
        }
    }

    return (m_logFile != nullptr);
}

QString QDebugLogger::getCurrentLogFileName()
{
    QString dateStr = QDate::currentDate().toString("yyyyMMdd");
    QString fileName = m_filePrefix.isEmpty() ? dateStr : m_filePrefix + "_" + dateStr;
    return m_logDir + QDir::separator() + fileName + ".log";
}

QString QDebugLogger::getLogLevelString(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return "DEBUG";
    case QtInfoMsg:
        return "INFO";
    case QtWarningMsg:
        return "WARNING";
    case QtCriticalMsg:
        return "CRITICAL";
    case QtFatalMsg:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

void QDebugLogger::outputToConsole(const QString &logMsg)
{
    cout << logMsg.toLocal8Bit().toStdString() << endl;
}

void QDebugLogger::handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // 检查日志级别
    if (type < m_logLevel)
        return;

    // 创建日志消息，只存储需要的上下文信息
    LogMessage logMsg(type, LogContext(context), msg);

    // 如果是异步模式，将消息放入队列
    if (m_async && m_loggerThread) {
        m_loggerThread->enqueueMessage(logMsg);
    } else {
        // 同步模式直接处理
        processLogMessage(logMsg);
    }
}

void QDebugLogger::processLogMessage(const LogMessage &msg)
{
    QMutexLocker locker(&m_mutex);

    // 确保日志文件存在
    if (!checkAndCreateLogFile())
        return;

    // 构建日志消息,长度不超过1024
    QString logMsg = QString("[%1] [%2] [%3:%4] %5")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
            .arg(getLogLevelString(msg.type))
            .arg(!msg.context.file.isEmpty() ? QFileInfo(msg.context.file).fileName() : "")
            .arg(msg.context.line)
            .arg(msg.message).left(1024);

    // 输出到控制台（如果启用）
    if (m_consoleOutput) {
        outputToConsole(logMsg);
    }

    // 写入日志文件
    *m_textStream << logMsg << "\n";
    m_textStream->flush();

    // 如果是致命错误，还需要调用abort()
    if (msg.type == QtFatalMsg) {
        abort();
    }
}

void LoggerThread::enqueueMessage(const LogMessage &msg) {
    QMutexLocker locker(&m_mutex);
    m_messageQueue.enqueue(msg);
    m_condition.wakeOne();
}

void LoggerThread::stop() {
    QMutexLocker locker(&m_mutex);
    m_stop = true;
    m_condition.wakeOne();
}

void LoggerThread::run() {
    while(1) {
        m_mutex.lock();
        while (m_messageQueue.isEmpty() && !m_stop)
            m_condition.wait(&m_mutex);

        if (m_stop)
            break;

        LogMessage msg = m_messageQueue.dequeue();
        m_mutex.unlock();

        //发送输出日志信息信号
        emit logMessageReady(msg);
    };
}
