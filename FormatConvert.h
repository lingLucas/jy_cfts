#ifndef FORMATCONVERT_H
#define FORMATCONVERT_H

#include <QImage>
#include "ItkCamera.h"

/* @brief：ITKBUFFER转QImage
 * @param[in] hBuffer：待转换的缓冲区结构体
 *
 * @brief：Convert ITKBUFFER to QImage
 * @param[in] hBuffer：The buffer need to convert */
QImage ImageFromItkBuffer(ItkCamera *cam, ITKBUFFER hBuffer);

/* @brief：获取采集卡的缓冲区数据，转为QImage
 * @param[in] cam：包含采集卡设备句柄的相机指针
 *
 * @brief：Get the buffer data of the grabber and convert to QImage
 * @param[in] cam：Camera struct pointer */
QImage ImageFromGrabber(ItkCamera *cam);

#endif // FORMATCONVERT_H
