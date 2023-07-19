#include "timethread.h"
#include <QDebug>


TimeThread::TimeThread(StatusServer* statusserver)
{
    m_statusserver = statusserver;
}

TimeThread::~TimeThread()
{

}

void TimeThread::stop()
{
    quit();
}

bool TimeThread::isDdePrinterExist()
{
    return m_statusserver->isHeart();
}

void TimeThread::run()
    {
        QTime t;
        t.start();
        qInfo() << "time run start.";
        while (1) {
            if (t.elapsed() > IDLEEXIT) {
                if (!isDdePrinterExist()) {
                    emit signalExitThread();
                    emit signalExitApp();
                    emit signalExitForwarder();
                    emit signalExitCupsMonitor();
                    break;
                }
                else {
                    t.restart();
                }
            }
            sleep(1);
        }
        stop();
    }

