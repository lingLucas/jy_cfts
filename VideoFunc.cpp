#include "VideoFunc.h"
#include "ImageConvert.h"

VideoFunc::VideoFunc(QObject *parent): QThread{parent}
{

}


void VideoFunc::setPath(const QString &path) {
    filePath = path;
}


void VideoFunc::run()
{
    if (filePath.isEmpty()) {
        QString msg = QString("video path is empty!");
        emit logMessage(msg);
        emit finished();
        return;
    }
    frameCount = 0;
    cap.open(filePath.toStdString());
    if (!cap.isOpened()) {
        QString msg = QString("无法打开视频:%1").arg(filePath);
        emit logMessage(msg);
        emit finished();
        return;
    }
    cv::Mat frame;
    while (cap.read(frame) && !isInterruptionRequested() ) {
        if (frame.empty()) break;

        // ✅ 这里可以插入检测逻辑，例如 YOLO 推理：
        // cv::Mat result = detect(frame);
        frameCount++;
        // QImage img = ImageConvert::MatToQImage(frame);
        emit sendImage(frame);
        QThread::msleep(30); // 控制播放速度 (约33ms对应30FPS)
    }
    cap.release();
    QString msg = QString("视频播放完成！");
    emit logMessage(msg);
    emit finished();
}

