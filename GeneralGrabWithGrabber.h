#pragma once

#include "ItkCamera.h"

/* @brief：配置采集卡设备。
 * @param[in] pCam：相机结构体指针。
 *
 * @brief：Configure frame grabber device.
 * @param[in] pCam：Camera struct pointer. */
bool ConfigureFrameGrabber(pItkCamera pCam);

/* @brief：开始图像采集。
 * @param[in] pCam：相机结构体指针。
 *
 * @brief：Start grabbing images.
 * @param[in] pCam：Camera struct pointer. */
bool StartGrabImage(pItkCamera pCam);

/* @brief：停止图像采集。
 * @param[in] pCam：相机结构体指针。
 *
 * @brief：Stop grabbing images.
 * @param[in] pCam：Camera struct pointer. */
bool StopGrabImage(pItkCamera pCam);
