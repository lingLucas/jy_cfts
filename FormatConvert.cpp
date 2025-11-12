#include "FormatConvert.h"
#include "ItkCamera.h"
#include <QDebug>

/// \~chinese   按双字节读取数据，截取高8位存到QImage中用作显示 \~english   Read data in two bytes, intercept high 8 bits into QImage for display
void ReadDataByShort(uchar* pDst, uchar* pSrc, qint64 srcSize, int nDepth)
{
    uchar* pReadCursor = nullptr;
    qint64 i = 0;
    int offset = nDepth - 8;
    for (i = 0; i < srcSize; i += 2) {
        pReadCursor = pSrc + i;
        *pDst++ = (pReadCursor[0] >> offset) | (pReadCursor[1] << (8 - offset));
    }
}

/// \~chinese  从8bit的RGBC数据中分离出RGB通道              	\~english Separate the RGB data from 8 bit RGBC data
void ReadRGBFromRGBC8(uchar* pSrc, qint64 iSrcSize, uchar* pDst)
{
    uchar* pReadCursor = NULL;
    qint64 i = 0;
    for (i = 0; i < iSrcSize; i += 4) {
        pReadCursor = pSrc + i;

        *pDst++ = pReadCursor[0];
        *pDst++ = pReadCursor[1];
        *pDst++ = pReadCursor[2];
    }
}

/// \~chinese  从16bit的RGBC数据中分离出RGB通道，并截取高8位      \~english Separate the RGB data from 16 bit RGBC data, intercept high 8 bits into QImage for display
void ReadRGB8FromRGBC16(uchar* pSrc, qint64 iSrcSize, uchar* pDst, int nDepth)
{
    uchar* pReadCursor = NULL;
    qint64 i = 0;
    int offset = nDepth - 8;
    for (i = 0; i < iSrcSize; i += 8) {
        pReadCursor = pSrc + i;

        *pDst++ = (pReadCursor[0] >> offset) | (pReadCursor[1] << (8 - offset));
        *pDst++ = (pReadCursor[2] >> offset) | (pReadCursor[3] << (8 - offset));
        *pDst++ = (pReadCursor[4] >> offset) | (pReadCursor[5] << (8 - offset));
    }
}

/// \~chinese  QImage释放时，释放分配的内存              	\~english Release the memory when QImage released
void cleanMalloc(void* info)
{
    free(info);
}

/// \~chinese  QImage释放时，释放传入的ITKBUFFER         	\~english Release the ITKBUFFER when QImage released
void cleanItkBuffer(void* info)
{
    if (nullptr == info)
        return;

    ITKBUFFER hBuffer = (ITKBUFFER)info;
    ItkBufferFree(hBuffer);
}
#include <QElapsedTimer>
/// \~chinese  ITKBUFFER转QImage         		        \~english Convert ITKBUFFER to QImage
QImage ImageFromItkBuffer(ItkCamera *cam, ITKBUFFER hBuffer)
{
    ITKBUFFER* hRead = &hBuffer;
    /// \~chinese  获取图像尺寸		                    \~english Get image size
    int64_t nImageWidth = 0;
    int64_t nImageHeight = 0;
    ItkBufferGetPrm(hBuffer, ITKBUFFER_PRM_WIDTH, &nImageWidth);
    ItkBufferGetPrm(hBuffer, ITKBUFFER_PRM_HEIGHT, &nImageHeight);

    /// \~chinese  判断是否需要转换		                    \~english Judge if need convert
    uint8_t bNeedCvt = 0;
    ItkBufferNeedAutoConvert(hBuffer, &bNeedCvt);

    if (bNeedCvt) {
        /// \~chinese  创建任意格式的缓冲区来存储转换结果		                    \~english Create buffer in any format as result
        QElapsedTimer timer;timer.start();
        ItkBufferConvert(hBuffer, cam->g_hCvtBuffer, 0, ITKBUFFER_VAL_CONVERT_OPTION_AUTO_FORMAT);qDebug() << timer.elapsed();
        hRead = &cam->g_hCvtBuffer;
    }

    /// \~chinese   获取像素深度和bit数来计算通道数   \~english Get pixel depth and bits to calculate channel
    int nImageDepth = 0, nDataBits = 0;
    ItkBufferGetPrm(*hRead, ITKBUFFER_PRM_PIXEL_DEPTH, &nImageDepth);
    ItkBufferGetPrm(*hRead, ITKBUFFER_PRM_DATA_BIT, &nDataBits);

    QImage::Format fmt;
    int channel = nDataBits / (nImageDepth == 8 ? 8 : 16);
    switch (channel)
    {
    case 1:
        fmt = QImage::Format_Grayscale8; break;
    case 3:
        fmt = QImage::Format_RGB888; break;
    case 4:
        fmt = QImage::Format_RGBA8888; break;
    default:
        return QImage();
    }

    /// \~chinese   从缓冲区读取数据    \~english   Read data from buffer
    void* pBufferAddr = nullptr;
    int64_t nImageSize = 0;
    ItkBufferGetPrm(*hRead, ITKBUFFER_PRM_ADDRESS, &pBufferAddr);
    ItkBufferGetPrm(*hRead, ITKBUFFER_PRM_SIZE, &nImageSize);

    if (nImageDepth > 8) {
        uchar* data = (uchar*)malloc(nImageSize / 2);
        ReadDataByShort(data, (uchar*)pBufferAddr, nImageSize, nImageDepth);
        return QImage(data, nImageWidth, nImageHeight, fmt, cleanMalloc, data);
    } else {
        return QImage((uchar*)pBufferAddr, nImageWidth, nImageHeight, fmt);
    }
}

/// \~chinese  获取采集卡的缓冲区数据，转为QImage		        \~english Get the buffer data of the grabber and convert to QImage
QImage ImageFromGrabber(ItkCamera *cam)
{
    /// \~chinese  获取图像尺寸		                    \~english Get image size
    int nImageWidth = 0;
    int nImageHeight = 0;
    IKapGetInfo(cam->g_hBoard, IKP_IMAGE_WIDTH, &nImageWidth);
    IKapGetInfo(cam->g_hBoard, IKP_IMAGE_HEIGHT, &nImageHeight);

    /// \~chinese   读取缓冲区数据                        \~english Read data from grabber
    int nFrameIndex = 0;
    uchar *pUserBuffer = NULL;
    int nImageSize = 0;
    IKapGetInfo(cam->g_hBoard, IKP_CURRENT_BUFFER_INDEX, &nFrameIndex);
    IKapGetInfo(cam->g_hBoard, IKP_FRAME_SIZE, &nImageSize);
    IKapGetBufferAddress(cam->g_hBoard, nFrameIndex, (void**)&pUserBuffer);

    /// \~chinese   判断是否存在C线                        \~english   Judge if extra channel exist
    int nImgType = 0;
    int nImgDepth = 0;
    IKapGetInfo(cam->g_hBoard, IKP_IMAGE_TYPE, &nImgType);
    IKapGetInfo(cam->g_hBoard, IKP_DATA_FORMAT, &nImgDepth);
    if (IKP_IMAGE_TYPE_VAL_RGBC == nImgType ||
        IKP_IMAGE_TYPE_VAL_BGRC == nImgType) {
        uchar* data = nullptr;
        if (nImgDepth == 8) {
            data = (uchar*)malloc(nImageSize * 0.75);
            ReadRGBFromRGBC8(pUserBuffer, nImageSize, data);
            return QImage(data, nImageWidth, nImageHeight, QImage::Format_RGB888, cleanMalloc, data);
        } else {
            data = (uchar*)malloc(nImageSize * 0.75 / 2);
            ReadRGB8FromRGBC16(pUserBuffer, nImageSize, data, nImgDepth);
            return QImage(data, nImageWidth, nImageHeight, QImage::Format_RGB888, cleanMalloc, data);
        }
    }

    /// \~chinese   创建缓冲区                           \~english Create buffer
    ITKBUFFER hBuffer = nullptr;
    uint32_t len = 128;
    char buffer[128] = {};
    uint32_t nFormat;
    ItkDevToString(cam->g_hCamera, "PixelFormat", buffer, &len);
    ItkDevGetPixelFormatVal(cam->g_hCamera, buffer, &nFormat);
    ItkBufferNewWithPtr(nImageWidth, nImageHeight, nFormat, pUserBuffer, &hBuffer);

    QImage img = ImageFromItkBuffer(cam, hBuffer);
    ItkBufferFree(hBuffer);
    return img;
}
