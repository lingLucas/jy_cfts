#ifndef VIDEOFUNC_H
#define VIDEOFUNC_H
#include <QThread>
#include <opencv2/opencv.hpp>
#include "DatabaseManager.h"


class VideoFunc : public QThread
{
    Q_OBJECT
public:
    explicit VideoFunc(QObject *parent = nullptr);
    void setPath(const QString &path);

protected:
    void run();

signals:
    void sendImage(const cv::Mat &image);
    void finished();
    void logMessage(const QString &msg);

private:
    QString filePath;
    cv::VideoCapture cap;
    int frameCount;
    bool stopFlag = false;
};

#endif // VIDEOFUNC_H
