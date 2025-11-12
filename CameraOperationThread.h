#ifndef CAMERAOPERATIONTHREAD_H
#define CAMERAOPERATIONTHREAD_H

#include <QThread>
#include <QVector>
#include "ItkCamera.h"

enum ITK_DEVICE_CONTROL_TYPE
{
    /// \~chinese 探测设备
    ITK_DEVICE_CONTROL_PROBE = 0,
    /// \~chinese 打开设备
    ITK_DEVICE_CONTROL_OPEN,
    /// \~chinese 关闭设备
    ITK_DEVICE_CONTROL_CLOSE,
    /// \~chinese 设备抓取
    ITK_DEVICE_CONTROL_GRAB,
    /// \~chinese 停止抓取
    ITK_DEVICE_CONTROL_STOP
};

/// \~chinese 操作相机的子线程
class DeviceControlThread : public QThread
{
    Q_OBJECT
public:
    explicit DeviceControlThread(QObject *parent = nullptr);
    virtual ~DeviceControlThread();

    void StartProbe();
    void StartOpen(uint32_t nIndex);
    void StartClose();
    void StartGrab();
    void StartStopGrab();

    QStringList GetDeviceList() const;

protected:
    void run();

    bool runProbe();
    bool runOpen();
    bool runClose();
    bool runGrab();
    bool runStopGrab();

    uint32_t mCamIndex;
    ITK_DEVICE_CONTROL_TYPE mCtrlType;
    /// \~chinese 扫描到的相机列表
    QVector<ItkCamera> mCameras;

signals:
    void signalControlFinish(ITK_DEVICE_CONTROL_TYPE nType, bool bSuccess);
    QString signalSelectGrabConfig();
    void signalImageGrabbed(const QImage& img);

    void logMessage(const QString &msg);

};

#endif // CAMERAOPERATIONTHREAD_H
