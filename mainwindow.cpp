#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QTableWidget"
#include "QHeaderView"
#include "QDateTime"
#include "QMessageBox"
#include "QPixmap"
#include "QFileDialog"
#include "ImageConvert.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initTableWidget();
    initUi();
}

MainWindow::~MainWindow()
{
    cleanVideoEvent();
    delete ui;
}


void MainWindow::initTableWidget()
{
    ui->tableWidget->setRowCount(25);
    ui->tableWidget->setColumnCount(4);
    // ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    QStringList headers = {"序号", "棒号", "坐标", "尺寸"};
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);

    ui->tableWidget->setColumnWidth(0, 40);
    ui->tableWidget->setColumnWidth(1, 50);
    ui->tableWidget->setColumnWidth(3, 50);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    ui->tableWidget->verticalHeader()->setVisible(false);

    ui->tableWidget->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { "
        "background-color: #87CEFA;"     // 蓝色
        "color: white;"                  // 白色文字
        "font: bold 12px 'Microsoft YaHei';"
        "height: 32px;"
        "border: none;"
        "}"
    );


}

void MainWindow::initUi()
{
    ui->actionDisconnect->setEnabled(false);
    ui->actionConnect->setEnabled(false);
    ui->actionOpen->setEnabled(false);
    ui->actionStop->setEnabled(false);
    mCtrlThread = new DeviceControlThread(this);

    algoThread = new QThread(this);
    imageAlgorithm = new ImageAlgorithm();
    imageAlgorithm->moveToThread(algoThread);
    connect(algoThread, &QThread::finished, imageAlgorithm, &QObject::deleteLater);
    connect(algoThread, &QThread::finished, algoThread, &QObject::deleteLater);
    algoThread->start();


    connect(ui->actionVideo, &QAction::triggered, this, &MainWindow::ActionVideoTriggered);
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::ActionConnectTriggered);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::ActionDisconnectTriggered);
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::ActionFindTriggered);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::ActionOpenTriggered);
    connect(ui->actionStop, &QAction::triggered, this, &MainWindow::ActionStopTriggered);


    connect(mCtrlThread, &DeviceControlThread::signalControlFinish, this, &MainWindow::onControlFinish);
    connect(mCtrlThread, &DeviceControlThread::logMessage, this, &MainWindow::logMessageOutput);
    connect(mCtrlThread, &DeviceControlThread::signalImageGrabbed, this, &MainWindow::displayQImage, Qt::BlockingQueuedConnection);

    // connect(videoFunc, &VideoFunc::finished, this, &MainWindow::logMessageOutput);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTimeDisplay);
    // connect(timer, &QTimer::timeout, this, &MainWindow::readFrame);
    timer->start(1000);
}

void MainWindow::cleanVideoEvent()
{
    if(videoFunc){
        videoFunc->requestInterruption();
        videoFunc->quit();
        videoFunc->wait();
        disconnect(videoFunc, nullptr, this, nullptr);
        videoFunc->deleteLater();
        videoFunc = nullptr; // 避免悬空指针
    }
    if (algoThread) {
        algoThread->quit();
        algoThread->wait();
    }
}

void MainWindow::updateTimeDisplay()
{
    // 获取当前时间并格式化
    QString timeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // 在 Label 上显示
    ui->label_time->setText(timeString);
}

void MainWindow::ActionFindTriggered()
{
    mCtrlThread->StartProbe();
    QThread::msleep(1000);
    logMessageOutput(QString("搜索相机完成!"));
}

void MainWindow::ActionConnectTriggered()
{
    int index = ui->comboBox_cameraList->currentIndex();
    mCtrlThread->StartOpen(index);
    QThread::msleep(1000);
    logMessageOutput(QString("连接相机!"));
}


void MainWindow::ActionDisconnectTriggered()
{
    if (gCamera) {
        mCtrlThread->StartClose();
    }
    logMessageOutput(QString("断开相机!"));
}


void MainWindow::ActionOpenTriggered()
{
    if (nullptr == gCamera) {
        QMessageBox::critical(this, "Critical", "connect camera first!");
        return;
    }

    if (!gCamera->g_bGrabbing) {
        mCtrlThread->StartGrab();
    }
    logMessageOutput(QString("打开相机!"));
}


void MainWindow::ActionStopTriggered()
{
    if (gCamera->g_bGrabbing) {
        mCtrlThread->StartStopGrab();
    }
    logMessageOutput(QString("关闭相机!"));
}


void MainWindow::onControlFinish(ITK_DEVICE_CONTROL_TYPE nType, bool bSuccess)
{
    switch (nType) {
    case ITK_DEVICE_CONTROL_PROBE:
    {
        if (!bSuccess) {
            QMessageBox::critical(this, "Critical", "Probe failed!");
            return;
        }
        QStringList camList = mCtrlThread->GetDeviceList();
        ui->comboBox_cameraList->clear();
        ui->comboBox_cameraList->addItems(camList);
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
        ui->actionOpen->setEnabled(false);
        ui->actionStop->setEnabled(false);
    }
    break;
    case ITK_DEVICE_CONTROL_OPEN:
        if (!bSuccess) {
            QMessageBox::critical(this, "Critical", "Failed to open camera!");
            return;
        }
        ui->comboBox_cameraList->setEnabled(false);
        ui->actionFind->setEnabled(false);

        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionOpen->setEnabled(true);
        ui->actionStop->setEnabled(false);

        break;
    case ITK_DEVICE_CONTROL_CLOSE:
        if (!bSuccess) {
            QMessageBox::critical(this, "Critical", "Failed to close camera!");
            return;
        }
        ui->actionFind->setEnabled(true);
        ui->actionConnect->setEnabled(true);
        ui->actionDisconnect->setEnabled(false);
        ui->actionOpen->setEnabled(false);
        ui->actionStop->setEnabled(false);
        ui->comboBox_cameraList->setEnabled(true);;
        break;
    case ITK_DEVICE_CONTROL_GRAB:
        if (!bSuccess) {
            QMessageBox::critical(this, "Critical", "Failed to start grab!");
            return;
        }
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(false);
        ui->actionOpen->setEnabled(false);
        ui->actionStop->setEnabled(true);
        break;
    case ITK_DEVICE_CONTROL_STOP:
        if (!bSuccess) {
            QMessageBox::critical(this, "Critical", "Failed to stop grab!");
            return;
        }
        ui->actionConnect->setEnabled(false);
        ui->actionDisconnect->setEnabled(true);
        ui->actionOpen->setEnabled(true);
        ui->actionStop->setEnabled(false);
        break;
    }
}

void MainWindow::displayOpenCVImage(const Mat &image)
{
    QImage qimg = ImageConvert::MatToQImage(image);

    // 创建QPixmap并显示
    QPixmap pixmap = QPixmap::fromImage(qimg);

    // 缩放图片以适应标签大小
    if (!pixmap.isNull()) {
        QSize labelSize = ui->imageshow->size();
        QPixmap scaledPixmap = pixmap.scaled(
            labelSize.width(), labelSize.height(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // ✅ 设置 QLabel 内容居中
        ui->imageshow->setAlignment(Qt::AlignCenter);
        ui->imageshow->setPixmap(scaledPixmap);
    }
}


void MainWindow::displayQImage(const QImage &qimg)
{
    // 创建QPixmap并显示
    QPixmap pixmap = QPixmap::fromImage(qimg);

    // 缩放图片以适应标签大小
    if (!pixmap.isNull()) {
        QSize labelSize = ui->imageshow->size();
        QPixmap scaledPixmap = pixmap.scaled(
            labelSize.width(), labelSize.height(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // ✅ 设置 QLabel 内容居中
        ui->imageshow->setAlignment(Qt::AlignCenter);
        ui->imageshow->setPixmap(scaledPixmap);
    }
}



void MainWindow::logMessageOutput(const QString &msg)
{
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->log_output->setFont(QFont("Consolas", 9));
    ui->log_output->appendPlainText(QString("[%1] %2").arg(time, msg));
}

void MainWindow::ActionVideoTriggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "打开视频",
                                                    "D:/项目/磁粉探伤/images/20251104/测试视频",
                                                    "视频文件 (*.mp4 *.h264)");
    if (fileName.isEmpty())
        return;
    if(videoFunc){
        videoFunc->requestInterruption();
        videoFunc->wait();
        disconnect(videoFunc, nullptr, this, nullptr);
    }
    videoFunc =  new VideoFunc(this);
    videoFunc->setPath(fileName);

    connect(videoFunc, &VideoFunc::sendImage, imageAlgorithm, &ImageAlgorithm::processImg, Qt::QueuedConnection);


    connect(videoFunc, &VideoFunc::logMessage, this, &MainWindow::logMessageOutput);
    connect(videoFunc, &VideoFunc::sendImage, this, &MainWindow::displayOpenCVImage, Qt::QueuedConnection);
    connect(videoFunc, &VideoFunc::finished, videoFunc, &QObject::deleteLater);
    connect(videoFunc, &VideoFunc::finished,
            this, [=](){
                videoFunc = nullptr;   // 避免悬空指针
                qDebug() << "视频线程安全退出";
            });


    videoFunc->start();
}

void MainWindow::readFrame()
{
    cv::Mat frame;
    if (!cap.read(frame) && cap.isOpened()) {
        timer->stop();
        cap.release();
        QMessageBox::information(this, "结束", "视频播放完毕！");
        return;
    }
    frameCount++;
    QImage qimg = ImageConvert::MatToQImage(frame);
    QString msg = QString("frame=%1 ,检测到缺陷数量%2").arg(frameCount).arg(20);
    // logMessageOutput(msg);
    qDebug()<<msg;
    QPixmap pix = QPixmap::fromImage(qimg);
    ui->imageshow->setAlignment(Qt::AlignCenter);
    ui->imageshow->setPixmap(pix.scaled(ui->imageshow->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

