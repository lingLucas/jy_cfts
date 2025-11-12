QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CameraOperationThread.cpp \
    DatabaseManager.cpp \
    FormatConvert.cpp \
    GeneralGrab.cpp \
    GeneralGrabWithGrabber.cpp \
    ImageAlgorithm.cpp \
    ImageConvert.cpp \
    VideoFunc.cpp \
    loggerfunc.cpp \
    loggerworker.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CameraOperationThread.h \
    DatabaseManager.h \
    FormatConvert.h \
    GeneralGrab.h \
    GeneralGrabWithGrabber.h \
    ImageAlgorithm.h \
    ImageConvert.h \
    ItkCamera.h \
    VideoFunc.h \
    loggerfunc.h \
    loggerworker.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


INCLUDEPATH += D:/alllib/opencv/build/include \
                $$quote(C:/Program Files/I-TEK OptoElectronics/IKapLibrary/Include)

LIBS += -L"D:/alllib/opencv/build/x64/vc16/lib"
CONFIG(debug, debug|release) {
    LIBS += -lopencv_world4110d
} else {
    LIBS += -lopencv_world4110
}

contains(QT_ARCH,i386){
    LIBS += -L"C:/Program Files/I-TEK OptoElectronics/IKapLibrary/Lib/x86/"
} else  {
    LIBS += -L"C:/Program Files/I-TEK OptoElectronics/IKapLibrary/Lib/x64/"
}
LIBS += -lIKapC -lIKapBoard
QMAKE_PROJECT_DEPTH = 0

RESOURCES += \
    resource.qrc
