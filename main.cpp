#include "mainwindow.h"

#include <QApplication>
#include "loggerfunc.h"
#include <QDateTime>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    QString setpath = "logs/" + QDateTime::currentDateTime().toString("yyyyMMdd")+".log";
    Logger::instance().setLogFilePath(setpath);
    Logger::instance().setMaxFileSize(10 * 1024 * 1024);
    Logger::instance().start();

    qInfo() << "主程序启动";
    w.setWindowTitle("磁粉探伤视觉系统");

    w.show();
    return a.exec();
}
