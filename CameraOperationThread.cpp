#include "CameraOperationThread.h"
// #include <QTextCodec>
#include <QDebug>
#include "GeneralGrab.h"
#include "GeneralGrabWithGrabber.h"
#include "FormatConvert.h"

pItkCamera gCamera = nullptr;

void CALLBACK OnFrameReady(void* pContext);

void IKAPC_CC cbOnEndOfFrame(uint32_t eventType, void* pContext);

DeviceControlThread::DeviceControlThread(QObject *parent)
    : QThread{parent}
{
    qRegisterMetaType<ITK_DEVICE_CONTROL_TYPE>("ITK_DEVICE_CONTROL_TYPE");
    ItkManInitialize();
    mCamIndex = 0;
    mCtrlType = ITK_DEVICE_CONTROL_PROBE;
}

DeviceControlThread::~DeviceControlThread()
{
    /// \~chinese  析构时判断相机是否关闭		                    \~english Judge if need close device when release
    if (gCamera) {
        StartClose();
        while (this->isRunning())
            msleep(10);
    }
    ItkManTerminate();
}

void DeviceControlThread::StartProbe()
{
    mCtrlType = ITK_DEVICE_CONTROL_PROBE;
    start();
}

void DeviceControlThread::StartOpen(uint32_t nIndex)
{
    mCamIndex = nIndex;
    mCtrlType = ITK_DEVICE_CONTROL_OPEN;
    start();
}

void DeviceControlThread::StartClose()
{
    mCtrlType = ITK_DEVICE_CONTROL_CLOSE;
    start();
}

void DeviceControlThread::StartGrab()
{
    mCtrlType = ITK_DEVICE_CONTROL_GRAB;
    start();
}

void DeviceControlThread::StartStopGrab()
{
    mCtrlType = ITK_DEVICE_CONTROL_STOP;
    start();
}

QStringList DeviceControlThread::GetDeviceList() const
{
    QStringList names;
    for (int i = 0; i < mCameras.size(); ++i)
        names << QString("%1 %2").arg(mCameras[i].g_devInfo.FullName).arg(mCameras[i].g_devInfo.SerialNumber);
    return names;
}

void DeviceControlThread::run()
{
    bool bSuccess = false;
    switch (mCtrlType) {
    case ITK_DEVICE_CONTROL_PROBE:
        bSuccess = runProbe();
        break;
    case ITK_DEVICE_CONTROL_OPEN:
        bSuccess = runOpen();
        break;
    case ITK_DEVICE_CONTROL_CLOSE:
        bSuccess = runClose();
        break;
    case ITK_DEVICE_CONTROL_GRAB:
        bSuccess = runGrab();
        break;
    case ITK_DEVICE_CONTROL_STOP:
        bSuccess = runStopGrab();
        break;
    }
    emit signalControlFinish(mCtrlType, bSuccess);
}

bool DeviceControlThread::runProbe()
{
    uint32_t res = ITKSTATUS_OK;
    uint32_t count = 0;
    ITKDEV_INFO info;

	mCameras.clear();
    /// \~chinese  获取设备个数		                    \~english Get the count of device
    res = ItkManGetDeviceCount(&count);
    if (res != ITKSTATUS_OK)
        return false;
    for (uint32_t i = 0; i < count; ++i) {
        /// \~chinese  获取设备信息		                \~english Get the information of device
        res = ItkManGetDeviceInfo(i, &info);
        if (res != ITKSTATUS_OK)
            continue;
        if (QString(info.SerialNumber).isEmpty())
            continue;

        ItkCamera cam;
        cam.g_nIndex = i;
        cam.g_devInfo = info;
        mCameras.append(cam);
    }
    return true;
}

bool DeviceControlThread::runOpen()
{
    uint32_t status = ITKSTATUS_OK;
    ItkCamera* cam = nullptr;
    if (mCameras.isEmpty() || mCamIndex >= mCameras.size())
        return false;

    cam = &mCameras[mCamIndex];
    status = ItkDevOpen(cam->g_nIndex, ITKDEV_VAL_ACCESS_MODE_EXCLUSIVE, &cam->g_hCamera);
    if (status != ITKSTATUS_OK)
        return false;

    gCamera = &mCameras[mCamIndex];

    ITK_CXP_DEV_INFO cxp_cam_info = {};
    ITK_GVB_DEV_INFO gvb_cam_info = {};
    ITK_CL_DEV_INFO  cl_cam_info  = {};
    void*            info         = NULL;
    if (strcmp(cam->g_devInfo.DeviceClass, "CoaXPress") == 0) {
        /// \~chinese 获取CoaXPress相机设备信息				    \~english Get CoaXPress camera device information
        status = ItkManGetCXPDeviceInfo(cam->g_nIndex, &cxp_cam_info);
        info = &cxp_cam_info;
    } else if (strcmp(cam->g_devInfo.DeviceClass, "GigEVisionBoard") == 0) {
        /// \~chinese 获取GigEVision相机设备信息				    \~english Get GigEVision camera device information
        status = ItkManGetGVBDeviceInfo(cam->g_nIndex, &gvb_cam_info);
        info = &gvb_cam_info;

    } else if (strcmp(cam->g_devInfo.DeviceClass, "CameraLink") == 0) {
        /// \~chinese 获取CameraLink相机设备信息				    \~english Get CameraLink camera device information
        status = ItkManGetCLDeviceInfo(cam->g_nIndex, &cl_cam_info);
        info = &cl_cam_info;
    } else {
        return true;
    }

    cam->g_hBoard = IKapOpenWithSpecificInfo(info);
    if (cam->g_hBoard == INVALID_HANDLE_VALUE)
        return false;

    /// \~chinese  连接采集卡的相机选择采集卡配置文件		     \~english Select configuration file if with grabber
    if (cam->g_hBoard != INVALID_HANDLE_VALUE) {
        QString vlcfFile = emit signalSelectGrabConfig();
        if (vlcfFile.isEmpty())
            return true;
        // QTextCodec* codec = QTextCodec::codecForName("GB2312");
        // QByteArray name = codec->fromUnicode(vlcfFile.toUtf8());
        // int ret = IKapLoadConfigurationFromFile(cam->g_hBoard, name.data());
        int ret = IKapLoadConfigurationFromFile(cam->g_hBoard, vlcfFile.toLocal8Bit().data());
        if (ret != IKStatus_Success) {
            return false;
        }
    }

    return true;
}

bool DeviceControlThread::runClose()
{
    uint32_t status = ITKSTATUS_OK;
    int ret = IKStatus_Success;

    /// \~chinese  关闭相机前先停图		                \~english Stop acquisition before close
    if (gCamera->g_bGrabbing) {
        if (!runStopGrab())
            return false;
    }

    status = ItkDevClose(gCamera->g_hCamera);
    if (status != ITKSTATUS_OK)
        return false;
    if (gCamera->g_hBoard != INVALID_HANDLE_VALUE) {
        ret = IKapClose(gCamera->g_hBoard);
        if (ret != IKStatus_Success)
            return false;
    }
    gCamera = nullptr;
    return true;
}

bool DeviceControlThread::runGrab()
{
    if (gCamera->g_hBoard != INVALID_HANDLE_VALUE) {
        if (!ConfigureFrameGrabber(gCamera))
            return false;
        int ret = IKapRegisterCallback(gCamera->g_hBoard, IKEvent_FrameReady, OnFrameReady, this);
        if (ret != IKStatus_Success)
            return false;
        if (!StartGrabImage(gCamera))
            return false;
    } else {
        if (!CreateStreamAndBuffer(gCamera))
            return false;
        if (!ConfigureStream(gCamera))
            return false;
        qDebug()<<"start";
        uint32_t res = ItkStreamRegisterCallback(gCamera->g_hStream, ITKSTREAM_VAL_EVENT_TYPE_END_OF_FRAME, cbOnEndOfFrame, this);
        qDebug()<<"end";
        if (res != ITKSTATUS_OK)
            return false;
        res = ItkStreamStart(gCamera->g_hStream, gCamera->g_grabCount);
        if (res != ITKSTATUS_OK)
            return false;
    }
    gCamera->g_bGrabbing = true;
    return true;
}

bool DeviceControlThread::runStopGrab()
{
    uint32_t res = ITKSTATUS_OK;
    int ret = IKStatus_Success;
    if (gCamera->g_hBoard != INVALID_HANDLE_VALUE) {
        ret = IKapUnRegisterCallback(gCamera->g_hBoard, IKEvent_FrameReady);
        if (ret != IKStatus_Success)
            return false;
        if (!StopGrabImage(gCamera))
            return false;
    } else {
        res = ItkStreamStop(gCamera->g_hStream);
        if (res != ITKSTATUS_OK)
            return false;
        res = ItkStreamUnregisterCallback(gCamera->g_hStream, ITKSTREAM_VAL_EVENT_TYPE_END_OF_FRAME);
        if (res != ITKSTATUS_OK)
            return false;
        res = ItkDevFreeStream(gCamera->g_hStream);
        if (res != ITKSTATUS_OK)
            return false;
    }
    res = ItkBufferFree(gCamera->g_hCvtBuffer);
    if (res != ITKSTATUS_OK)
        return false;
    gCamera->g_bGrabbing = false;
    return true;
}

void CALLBACK OnFrameReady(void* pContext)
{
    qDebug() << QString("Grab frame ready of camera with serialNumber:%1.").arg(QString(gCamera->g_devInfo.SerialNumber));
    int nFrameIndex = 0;
    IKAPBUFFERSTATUS status;
    DeviceControlThread* thread = (DeviceControlThread*)pContext;

    IKapGetInfo(gCamera->g_hBoard, IKP_CURRENT_BUFFER_INDEX, &nFrameIndex);
    IKapGetBufferStatus(gCamera->g_hBoard, nFrameIndex, &status);

    if (status.uFull == 1) {
        QImage img = ImageFromGrabber(gCamera);
        emit thread->signalImageGrabbed(img);
    }

    // 手动释放缓冲区
    // Release the buffer manual
    IKapReleaseBuffer(gCamera->g_hBoard, nFrameIndex);
}

void IKAPC_CC cbOnEndOfFrame(uint32_t eventType, void* pContext)
{
    ITKBUFFER  hBuffer = NULL;
    uint32_t bStatus = 0;
    uint64_t nBlockID = 0;
    DeviceControlThread* thread = (DeviceControlThread*)pContext;

    ItkStreamGetPrm(gCamera->g_hStream, ITKSTREAM_PRM_CURRENT_BUFFER_HANDLE, &hBuffer);
    ItkBufferGetPrm(hBuffer, ITKBUFFER_PRM_BLOCK_ID, &nBlockID);

    qDebug() << QString("On end of frame of camera with serialNumber:%1, block id: %2.").arg(QString(gCamera->g_devInfo.SerialNumber)).arg(nBlockID);
    // emit thread->logMessage(QString("frame id : %1").arg(nBlockID));
    // QString logMsg = QString("On end of frame of camera with serialNumber:%1, block id: %2.")
    //                      .arg(QString(gCamera->g_devInfo.SerialNumber))
    //                      .arg(nBlockID);
    // emit thread->logMessage(logMsg);
    ItkBufferGetPrm(hBuffer, ITKBUFFER_PRM_STATE, &bStatus);
    if (bStatus == ITKBUFFER_VAL_STATE_FULL) {
        QImage img = ImageFromItkBuffer(gCamera, hBuffer);
        emit thread->signalImageGrabbed(img);
    } else if (bStatus == ITKBUFFER_VAL_STATE_UNCOMPLETED) {
        qWarning() << QObject::tr("Frame uncompleted!");
    } else if (bStatus == ITKBUFFER_VAL_STATE_EMPTY)
        qWarning() << QObject::tr("Frame empty!");
    else
        qWarning() << QObject::tr("Frame overflow!");

    // 手动释放缓冲区
    ItkStreamClearBuffer(gCamera->g_hStream, hBuffer);
}
