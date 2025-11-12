#ifndef IMAGECONVERT_H
#define IMAGECONVERT_H

#include <opencv2/opencv.hpp>
#include <QImage>
#include <QDebug>
#include <QThread>

using namespace cv;

class ImageConvert
{
public:
    explicit ImageConvert();

    static Mat QImageToMat(const QImage &image, bool cloneImageData = true);
    static QImage MatToQImage(const cv::Mat& mat);
};

#endif // IMAGECONVERT_H

