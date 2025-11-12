#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QTimer"
#include "CameraOperationThread.h"
#include "VideoFunc.h"
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include "ImageAlgorithm.h"

using namespace cv;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void ActionConnectTriggered();

    void ActionDisconnectTriggered();

    void ActionFindTriggered();

    void ActionOpenTriggered();

    void ActionStopTriggered();

    void updateTimeDisplay();

    void onControlFinish(ITK_DEVICE_CONTROL_TYPE nType, bool bSuccess);

    void displayOpenCVImage(const Mat &image);

    void displayQImage(const QImage &qimg);

    void logMessageOutput(const QString &msg);

    void readFrame();


    void ActionVideoTriggered();

private:  
    void initTableWidget();
    void initUi();
    void cleanVideoEvent();


    Ui::MainWindow *ui;
    QTimer *timer;
    cv::VideoCapture cap;
    DeviceControlThread* mCtrlThread;
    int frameCount;
    VideoFunc *videoFunc = nullptr;
    ImageAlgorithm *imageAlgorithm;
    QThread *algoThread;
};
#endif // MAINWINDOW_H
