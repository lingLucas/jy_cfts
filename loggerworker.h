#pragma once

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QQueue>
#include <QMutex>
#include <QTimer>
#include <QDateTime>

class LoggerWorker : public QObject
{
    Q_OBJECT

public:
    explicit LoggerWorker(const QString& path, qint64 maxSize, QObject *parent = nullptr);
    ~LoggerWorker();

public slots:
    // 对外接口，用于接收日志消息
    void enqueue(const QString& msg);
    // 启动处理循环的槽
    void process();
    // 停止处理循环的槽
    void stop();

signals:
    // 用于向外报告错误
    void error(const QString& errorMessage);

private slots:
    // 定时器触发的槽，用于处理队列中的消息
    void processQueue();

private:
    void openFile();
    void writeLine(const QString& line);
    void rotateIfNeeded();

private:
    QString m_path;
    qint64 m_maxSize;
    QFile m_file;
    QTextStream m_stream;

    QQueue<QString> m_queue;
    QMutex m_mutex;
    QTimer* m_timer;
    bool m_stop = false;
};
