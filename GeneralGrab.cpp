#include "GeneralGrab.h"

/// \~chinese 创建数据流和缓冲区				\~english Create data stream and buffer
bool CreateStreamAndBuffer(pItkCamera pCam)
{
    ITKSTATUS res = ITKSTATUS_OK;

    /// \~chinese  申请数据流资源		            \~english Allocate data stream source
    res = ItkDevAllocStreamEx(pCam->g_hCamera, 0, pCam->g_bufferCount, &pCam->g_hStream);
    if (res != ITKSTATUS_OK)
        return false;

    /// \~chinese  创建格式转换的缓冲区	            \~english Create buffer for format convert
    int64_t nWidth = 0, nHeight = 0;
    res = ItkDevGetInt64(pCam->g_hCamera, "Width", &nWidth);
    if (res != ITKSTATUS_OK)
        return false;
    res = ItkDevGetInt64(pCam->g_hCamera, "Height", &nHeight);
    if (res != ITKSTATUS_OK)
        return false;
    res = ItkBufferNew(nWidth, nHeight, ITKBUFFER_VAL_FORMAT_BGR161616, &pCam->g_hCvtBuffer);
    if (res != ITKSTATUS_OK)
        return false;

    return true;
}

/// \~chinese  配置数据流		                    \~english Configure data stream
bool ConfigureStream(pItkCamera pCam)
{
    ITKSTATUS res = ITKSTATUS_OK;

    /// \~chinese  传输模式		                \~english Transfer mode
    uint32_t xferMode = ITKSTREAM_VAL_TRANSFER_MODE_SYNCHRONOUS_WITH_PROTECT;

    /// \~chinese  采集模式		                \~english Grab mode
    uint32_t startMode = ITKSTREAM_VAL_START_MODE_NON_BLOCK;

    /// \~chinese  超时时间		                \~english Time out time
    uint32_t timeOut = INFINITE;

    /// \~chinese  禁用自动清空缓冲区           \~english Disable auto clear buffer
    uint32_t autoClear = 0;

    /// \~chinese  设置采集模式		            \~english Set grab mode
    res = ItkStreamSetPrm(pCam->g_hStream, ITKSTREAM_PRM_START_MODE, &startMode);
    if (res != ITKSTATUS_OK)
        return false;

    /// \~chinese  设置传输模式		            \~english Set transfer mode
    res = ItkStreamSetPrm(pCam->g_hStream, ITKSTREAM_PRM_TRANSFER_MODE, &xferMode);
    if (res != ITKSTATUS_OK)
        return false;

    /// \~chinese  设置超时时间		            \~english Set time out time
    res = ItkStreamSetPrm(pCam->g_hStream, ITKSTREAM_PRM_TIME_OUT, &timeOut);
    if (res != ITKSTATUS_OK)
        return false;

    /// \~chinese  设置禁用自动清空             \~english Set disable auto clear
    res = ItkStreamSetPrm(pCam->g_hStream, ITKSTREAM_PRM_AUTO_CLEAR, &autoClear);
    if (res != ITKSTATUS_OK)
        return false;

    return true;
}
