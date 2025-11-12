#pragma once

#include <Windows.h>

#include <IKapBoard.h>
#include <IKapC.h>

typedef struct ItkCamera {
    /// \~chinese 相机索引                      \~english Index of camera
    uint32_t g_nIndex = -1;

    /// \~chinese 图像缓冲区申请的帧数				\~english The number of frames requested by buffer
    int g_bufferCount = 5;

    /// \~chinese 希望采集的帧数						\~english Number of frames wanted
    uint32_t g_grabCount = ITKSTREAM_CONTINUOUS;

    /// \~chinese 是否开启软触发						\~english Whether enable softTrigger or not
    uint8_t g_bSoftTriggerUsed = 0;

    /// \~chinese 是否加载采集卡配置文件				\~english Whether load grabber configure file or not
    uint8_t g_bLoadGrabberConfig = 0;

    /// \~chinese 是否正在采集                    \~english Whether grabbing
    uint8_t g_bGrabbing = 0;

    /// \~chinese 相机设备句柄						\~english Camera device handle
    ITKDEVICE g_hCamera = NULL;

    /// \~chinese 数据流句柄						\~english Data stream handle
    ITKSTREAM g_hStream = NULL;

    /// \~chinese 相机设备信息						\~english Camera device info
    ITKDEV_INFO g_devInfo;

    /// \~chinese 采集卡设备句柄						\~english Frame grabber device handle
    HANDLE g_hBoard = INVALID_HANDLE_VALUE;

    /// \~chinese 用于储存转换结果的缓冲区句柄        \~english Convert result buffer handle
    ITKBUFFER g_hCvtBuffer = NULL;

} *pItkCamera;

extern pItkCamera gCamera;
