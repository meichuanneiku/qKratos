#ifndef QDEBUGLOGGER_H
#define QDEBUGLOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QMutex>
#include <QDebug>
#include <QQueue>
#include <QThread>
#include <QWaitCondition>
#include <QMetaType>  // 新增：元对象系统头文件

// 存储必要的日志上下文信息
struct LogContext {
    QString file;
    int line;
    QString function;

    LogContext() : line(0) {}
    LogContext(const QMessageLogContext &context)
        : file(context.file ? context.file : ""),
          line(context.line),
          function(context.function ? context.function : "") {}
};

// 日志消息结构体
struct LogMessage {
    QtMsgType type;
    LogContext context;
    QString message;

    LogMessage() : type(QtDebugMsg) {}
    LogMessage(QtMsgType t, const LogContext &ctx, const QString &msg)
        : type(t), context(ctx), message(msg) {}
};

// 新增：声明LogMessage为元类型
Q_DECLARE_METATYPE(LogContext)
Q_DECLARE_METATYPE(LogMessage)

class LoggerThread : public QThread
{
    Q_OBJECT
public:
    explicit LoggerThread(QObject *parent = nullptr) : QThread(parent), m_stop(false) {}
    ~LoggerThread() override { stop(); wait(); }

    void enqueueMessage(const LogMessage &msg);

    void stop();

signals:
    void logMessageReady(const LogMessage &msg);

protected:
    void run() override;

private:
    QQueue<LogMessage> m_messageQueue;
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_stop;
};

class QDebugLogger : public QObject
{
    Q_OBJECT
public:
    static QDebugLogger& instance(){
        static QDebugLogger logger;
        return logger;
    }
    ~QDebugLogger();

    /**
     * @brief init              初始化日志系统
     * @param logDir            日志保存路径
     * @param prefix            日志名称前缀
     * @param maxSize           日志文件大小最大值
     * @param logLevel          日志输出最小级别
     * @param consoleOutput     是否输出到控制台
     * @param async             是否同步
     */
    void init();

    // 设置日志级别
    void setLogLevel(QtMsgType level);

    // 设置单个日志文件的最大大小(字节)
    void setMaxFileSize(qint64 size);

    // 设置日志存储目录
    void setLogDirectory(const QString& dir);

    // 设置日志文件名前缀
    void setFileNamePrefix(const QString& prefix);

    // 设置是否输出到控制台
    void setConsoleOutput(bool enabled);

    // 设置是否使用异步模式
    void setAsyncMode(bool async);

private slots:
    // 处理日志消息
    void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    // 实际处理日志写入（可在工作线程中调用）
    void processLogMessage(const LogMessage &msg);

private:
    QDebugLogger() = default;
//    QDebugLogger(const QDebugLogger&) = delete;
    QDebugLogger& operator=(const QDebugLogger&) = delete;

    // 确保日志目录存在
    bool ensureLogDirectory();

    // 检查并创建新的日志文件
    bool checkAndCreateLogFile();

    // 获取当前应该使用的日志文件名
    QString getCurrentLogFileName();

    // 获取日志级别字符串表示
    QString getLogLevelString(QtMsgType type);

    // 输出到控制台
    void outputToConsole(const QString &logMsg);

private:
    static QDebugLogger* m_instance;
    static QMutex m_mutex;

    QFile* m_logFile;
    QTextStream* m_textStream;
    QString m_logDir;
    QString m_filePrefix;
    qint64 m_maxSize;
    QtMsgType m_logLevel;
    QString m_currentFileName;
    bool m_consoleOutput;
    bool m_async;

    LoggerThread* m_loggerThread = 0;
};


#endif // QDEBUGLOGGER_H
