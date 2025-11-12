#include "ImageConvert.h"


ImageConvert::ImageConvert() {}



//Mat转图像
QImage ImageConvert::MatToQImage(const cv::Mat& mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if (mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for (int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for (int row = 0; row < mat.rows; row++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if (mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if (mat.type() == CV_8UC4)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        return QImage();
    }
}


Mat ImageConvert::QImageToMat(const QImage &image, bool cloneImageData)
{
    switch (image.format())
    {
    // 8-bit, 4 channel (RGBA)
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
    {
        cv::Mat mat(image.height(), image.width(),
                    CV_8UC4,
                    (void*)image.constBits(),
                    image.bytesPerLine());
        return (cloneImageData ? mat.clone() : mat);
    }

    // 8-bit, 3 channel (RGB888)
    case QImage::Format_RGB32:
    case QImage::Format_RGB888:
    {
        QImage swapped = image;
        if (image.format() == QImage::Format_RGB888)
            swapped = image.rgbSwapped(); // Qt是RGB，OpenCV默认BGR

        cv::Mat mat(swapped.height(), swapped.width(),
                    CV_8UC3,
                    (void*)swapped.constBits(),
                    swapped.bytesPerLine());
        return (cloneImageData ? mat.clone() : mat);
    }

    // 8-bit, 1 channel (grayscale)
    case QImage::Format_Indexed8:
    {
        cv::Mat mat(image.height(), image.width(),
                    CV_8UC1,
                    (void*)image.constBits(),
                    image.bytesPerLine());
        return (cloneImageData ? mat.clone() : mat);
    }

    default:
        qWarning() << "QImageToCvMat() - Unsupported QImage format:"
                   << image.format();
        return cv::Mat();
    }
}

