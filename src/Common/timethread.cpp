#include "timethread.h"
#include <QDebug>


TimeThread::TimeThread()
{

}

TimeThread::~TimeThread()
{

}

void TimeThread::stop()
{
    quit();
}

void TimeThread::run()
    {
        QTime t;
        t.start();
        qInfo() << "time run start.";
        while (1) {
            if (t.elapsed() > IDLEEXIT) {
                emit signalExitThread();
                emit signalExitApp();
                emit signalExitForwarder();
                emit signalExitCupsMonitor();
                break;
            }
            sleep(1);
        }
        stop();
    }

