#include "loggerfunc.h"
#include "loggerworker.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QMetaObject>
#include <QDebug>

// 全局消息处理器
static QtMessageHandler s_oldHandler = nullptr;

void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    QString levelStr;
    Logger::Level level;
    switch (type) {
    case QtDebugMsg:    levelStr = "DEBUG"; level = Logger::Debug; break;
    case QtInfoMsg:     levelStr = "INFO";  level = Logger::Info; break;
    case QtWarningMsg:  levelStr = "WARN";  level = Logger::Warning; break;
    case QtCriticalMsg: levelStr = "ERROR"; level = Logger::Error; break;
    case QtFatalMsg:    levelStr = "FATAL"; level = Logger::Error; break;
    }

    // QString file = QFileInfo(context.file ? context.file : "").fileName();
    // QString formatted = QString("[%1] %2: %3 (%4:%5)")
    //                         .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"))
    //                         .arg(levelStr)
    //                         .arg(msg)
    //                         .arg(file)
    //                         .arg(context.line);

    QString formatted = QString("[%1] %2: %3")
                            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"), levelStr, msg);

    // 发送给我们的日志系统
    Logger::instance().log(level, formatted);

    // 如果是致命错误，则中止
    if (type == QtFatalMsg) {
        abort();
    }
}

// ---------------- Logger 实现 ----------------

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger(QObject *parent) : QObject(parent) {
    // 可以根据应用名称设置默认日志路径
    if (QCoreApplication::instance()) {
        m_path = QCoreApplication::applicationName() + ".log";
    }
}

Logger::~Logger() {
    stop();
}

void Logger::setLogFilePath(const QString& path) {
    m_path = path;
}

void Logger::setMaxFileSize(qint64 bytes) {
    m_maxSize = bytes;
}

void Logger::start() {
    if (m_thread && m_thread->isRunning()) {
        return; // 已经启动
    }

    m_thread = new QThread(this);
    m_worker = new LoggerWorker(m_path, m_maxSize);
    m_worker->moveToThread(m_thread);

    // 连接信号和槽
    connect(m_thread, &QThread::started, m_worker, &LoggerWorker::process);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &Logger::newLogEnqueued, m_worker, &LoggerWorker::enqueue, Qt::QueuedConnection);
    connect(m_worker, &LoggerWorker::error, [](const QString& err) {
        qWarning() << "Logger Error:" << err;
    });

    m_thread->start();

    // 安装全局消息处理器
    s_oldHandler = qInstallMessageHandler(qtMessageHandler);
}

void Logger::stop() {
    if (s_oldHandler) {
        qInstallMessageHandler(s_oldHandler);
        s_oldHandler = nullptr;
    }

    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, &LoggerWorker::stop, Qt::QueuedConnection);
    }

    if (m_thread) {
        m_thread->quit();
        if (!m_thread->wait(5000)) { // 等待5秒
            qWarning() << "Logger thread did not stop, terminating.";
            m_thread->terminate();
            m_thread->wait();
        }
        m_thread = nullptr;
        m_worker = nullptr;
    }
}

void Logger::log(Level level, const QString& msg) {
    Q_UNUSED(level);
    if (!m_worker) {
        // 如果日志器未启动，则输出到控制台
        // fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
        qDebug()<<msg.toLocal8Bit().constData();
        return;
    }
    // 通过信号将日志发送给后台线程
    emit newLogEnqueued(msg);
}
