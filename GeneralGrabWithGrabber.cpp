#include "GeneralGrabWithGrabber.h"

/// \~chinese   配置采集卡设备       \~english   Configure frame grabber device
bool ConfigureFrameGrabber(pItkCamera pCam)
{
    int       ret = IK_RTN_OK;

    /// \~chinese   设置缓冲区个数         \~english   Set buffer count
    ret = IKapSetInfo(pCam->g_hBoard, IKP_FRAME_COUNT, pCam->g_bufferCount);
    if (ret != IKStatus_Success)
        return false;

    /// \~chinese   设置采集超时时间        \~english   Set time of timeout
    int timeout = -1;
    ret         = IKapSetInfo(pCam->g_hBoard, IKP_TIME_OUT, timeout);
    if (ret != IKStatus_Success)
        return false;

    /// \~chinese   设置采集模式为非阻塞      \~english   Set grab mode as non-block
    int grab_mode = IKP_GRAB_NON_BLOCK;
    ret           = IKapSetInfo(pCam->g_hBoard, IKP_GRAB_MODE, grab_mode);
    if (ret != IKStatus_Success)
        return false;

    /// \~chinese   设置传输模式为同步保护传输   \~english   Set transfer mode as synchronous and protect
    int transfer_mode = IKP_FRAME_TRANSFER_SYNCHRONOUS_NEXT_EMPTY_WITH_PROTECT;
    ret               = IKapSetInfo(pCam->g_hBoard, IKP_FRAME_TRANSFER_MODE, transfer_mode);
    if (ret != IKStatus_Success)
        return false;

    /// \~chinese   禁用缓冲区自动清空       \~english   Disable auto clear buffer function
    int auto_clear = IKP_FRAME_AUTO_CLEAR_VAL_DISABLE;
    ret               = IKapSetInfo(pCam->g_hBoard, IKP_FRAME_AUTO_CLEAR, auto_clear);
    if (ret != IKStatus_Success)
        return false;

    return true;
}

/// \~chinese   开始图像采集       \~english   Start grabbing images
bool StartGrabImage(pItkCamera pCam)
{
    ITKSTATUS res = ITKSTATUS_OK;
    int       ret = IK_RTN_OK;

    /// \~chinese  创建格式转换的缓冲区	            \~english Create buffer for format convert
    int nWidth = 0, nHeight = 0;
    ret = IKapGetInfo(pCam->g_hBoard, IKP_IMAGE_WIDTH, &nWidth);
    if (ret != IKStatus_Success)
        return false;
    ret = IKapGetInfo(pCam->g_hBoard, IKP_IMAGE_HEIGHT, &nHeight);
    if (ret != IKStatus_Success)
        return false;
    res = ItkBufferNew(nWidth, nHeight, ITKBUFFER_VAL_FORMAT_BGR161616, &pCam->g_hCvtBuffer);
    if (ITKSTATUS_OK != res)
        return false;

    if (strcmp(pCam->g_devInfo.DeviceClass, "CameraLink") != 0) {
        res = ItkDevExecuteCommand(pCam->g_hCamera, "AcquisitionStop");
        if (ITKSTATUS_OK != res)
            return false;
    }

    ret = IKapStartGrab(pCam->g_hBoard, 0);
    if (ret != IKStatus_Success)
        return false;

    if (strcmp(pCam->g_devInfo.DeviceClass, "CameraLink") != 0) {
        res = ItkDevExecuteCommand(pCam->g_hCamera, "AcquisitionStart");
        if (ITKSTATUS_OK != res)
            return false;
    }

    return true;
}

/// \~chinese   停止图像采集       \~english   Stop grabbing images
bool StopGrabImage(pItkCamera pCam)
{
    ITKSTATUS res = ITKSTATUS_OK;
    int       ret = IK_RTN_OK;

    if (strcmp(pCam->g_devInfo.DeviceClass, "CameraLink") != 0) {
        res = ItkDevExecuteCommand(pCam->g_hCamera, "AcquisitionStop");
        if (ITKSTATUS_OK != res)
            return false;
    }

    ret = IKapStopGrab(pCam->g_hBoard);
    if (ret != IKStatus_Success)
        return false;

    return true;
}
