#include "ImageAlgorithm.h"
#include <QDebug>

ImageAlgorithm::ImageAlgorithm(QObject *parent)
    : QObject{parent}
{}


void ImageAlgorithm::processImg(const cv::Mat &frame)
{
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    emit sendFrameResult(gray);
    qDebug() << "VideoFunc emit image, size=" << gray.cols << gray.rows;
}
