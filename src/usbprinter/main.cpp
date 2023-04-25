#include "usbthread.h"
#include "signalforwarder.h"
#include <QDebug>
#include <QApplication>

int main(int argc, char *argv[])
{
    //qInfo() << "主线程对象的地址是：" << QThread::currentThread();
    QApplication app(argc,argv);
    USBThread mythread;
    mythread.start();
    SignalForwarder forwarder;
    //QObject::connect(&usbThread, &USBThread::deviceStatusChanged, &forwarder, &SignalForwarder::slotDeviceStatusChanged);
    QThread forwarderThread;
    forwarder.moveToThread(&forwarderThread);
    forwarderThread.start();
    return app.exec();
}