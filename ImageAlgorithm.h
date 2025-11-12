#ifndef IMAGEALGORITHM_H
#define IMAGEALGORITHM_H

#include <QObject>
#include <opencv2/opencv.hpp>
class ImageAlgorithm : public QObject
{
    Q_OBJECT
public:
    explicit ImageAlgorithm(QObject *parent = nullptr);

public slots:

    void processImg(const cv::Mat &frame);

signals:
    void sendFrameResult(const cv::Mat &frame);

signals:
};

#endif // IMAGEALGORITHM_H
