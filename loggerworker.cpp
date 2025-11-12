#include "loggerworker.h"
#include <QDir>
#include <QDebug>

LoggerWorker::LoggerWorker(const QString& path, qint64 maxSize, QObject *parent)
    : QObject(parent)
    , m_path(path)
    , m_maxSize(maxSize)
{
    // 使用定时器来驱动队列处理，避免阻塞事件循环
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &LoggerWorker::processQueue);
}

LoggerWorker::~LoggerWorker() {
    if (m_file.isOpen()) {
        m_file.close();
    }
}

void LoggerWorker::process() {
    qDebug() << "LoggerWorker thread started.";
    openFile();
    // 启动定时器，每10毫秒检查一次队列
    m_timer->start(10);
}

void LoggerWorker::stop() {
    qDebug() << "LoggerWorker stopping...";
    m_timer->stop();
    m_stop = true;
    // 最后处理一次剩余的消息
    processQueue();
}

void LoggerWorker::enqueue(const QString& msg) {
    QMutexLocker locker(&m_mutex);
    m_queue.enqueue(msg);
}

void LoggerWorker::processQueue() {
    QMutexLocker locker(&m_mutex);
    while (!m_queue.isEmpty()) {
        QString msg = m_queue.dequeue();
        locker.unlock(); // 解锁再进行文件I/O，避免长时间占用锁
        writeLine(msg);
        locker.relock(); // 重新加锁以继续检查队列
    }
}

void LoggerWorker::openFile() {
    if (m_path.isEmpty()) return;

    // 确保目录存在
    QDir().mkpath(QFileInfo(m_path).absolutePath());

    m_file.setFileName(m_path);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        emit error(QString("Failed to open log file '%1': %2").arg(m_path, m_file.errorString()));
        return;
    }
    m_stream.setDevice(&m_file);
    qDebug() << "Log file opened:" << m_path;
}

void LoggerWorker::writeLine(const QString& line) {
    if (!m_stream.device()) {
        return;
    }
    rotateIfNeeded();
    m_stream << line << Qt::endl; // Qt::endl 会自动刷新
}

void LoggerWorker::rotateIfNeeded() {
    if (m_path.isEmpty()) return;

    if (m_file.size() >= m_maxSize) {
        m_file.close();

        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        QFileInfo fi(m_path);
        QString newPath = QString("%1/%2_%3.%4")
                              .arg(fi.path(), fi.completeBaseName(), timestamp, fi.suffix());

        if (QFile::rename(m_path, newPath)) {
            qDebug() << "Log rotated to:" << newPath;
        } else {
            emit error(QString("Failed to rotate log file to '%1'").arg(newPath));
        }

        // 重新打开新文件
        openFile();
    }
}
