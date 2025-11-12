#pragma once

#include <QObject>
#include <QString>
#include <QMutex>
#include <QThread>

class LoggerWorker; // 前向声明

class Logger : public QObject
{
    Q_OBJECT
public:
    enum Level { Debug, Info, Warning, Error };

    // 单例访问点
    static Logger& instance();

    // 配置接口
    void setLogFilePath(const QString& path);
    void setMaxFileSize(qint64 bytes);

    // 启动和停止
    void start();
    void stop();

    // 日志记录接口
    void log(Level level, const QString& msg);
    void debug(const QString& msg) { log(Debug, msg); }
    void info(const QString& msg)  { log(Info, msg); }
    void warn(const QString& msg)  { log(Warning, msg); }
    void error(const QString& msg) { log(Error, msg); }

signals:
    void newLogEnqueued(const QString& formatted); // 用于发送给 worker

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();

    // 禁用拷贝构造和赋值
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // 友元函数，用于处理 Qt 的全局日志
    friend void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    QString m_path = "app.log";
    qint64 m_maxSize = 10 * 1024 * 1024; // 默认 10MB

    LoggerWorker* m_worker = nullptr;
    QThread* m_thread = nullptr;
};
