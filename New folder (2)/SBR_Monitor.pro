TARGET = qmodbus
TEMPLATE = app
VERSION = 0.1.0

QT       += gui widgets
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    3rdparty/libmodbus/src/modbus-data.c \
    3rdparty/libmodbus/src/modbus-tcp.c \
    3rdparty/libmodbus/src/modbus.c \
    ipaddressctrl.cpp \
    iplineedit.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp

HEADERS += \
    3rdparty/libmodbus/src/modbus.h \
    imodbus.h \
    ipaddressctrl.h \
    iplineedit.h \
    mainwindow.h \
    qcustomplot.h

INCLUDEPATH += 3rdparty/libmodbus \
               3rdparty/libmodbus/src \
               3rdparty/qextserialport \
               src

unix {
    SOURCES +=
    DEFINES += _TTY_POSIX_
}

win32 {
    SOURCES +=
    DEFINES += _TTY_WIN_  WINVER=0x0501
    LIBS += -lsetupapi -lws2_32
}



FORMS += \
    ipaddressctrl.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += data/qmodbus.qrc

RC_FILE += qmodbus.rc

include(deployment.pri)
