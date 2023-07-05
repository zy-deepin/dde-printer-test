#include "signalusb.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //qInfo() << "主线程对象的地址是：" << QThread::currentThread();
    QApplication app(argc,argv);
    SignalUSB mythread;
    mythread.start();
    return app.exec();
}