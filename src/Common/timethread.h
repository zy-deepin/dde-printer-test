#ifndef TIMETHREAD_H
#define TIMETHREAD_H

#include "statusserver.h"
#include <QThread>
#include <QTime>
#include <pthread.h>

#define IDLEEXIT 1000 * 60 * 5

class TimeThread : public QThread
{
    Q_OBJECT
public:
    TimeThread(StatusServer* statusserver);
    ~TimeThread();
protected:
    void run() override;
    void stop();

    //判断dde-printer进程是否存在.
    bool isDdePrinterExist();
signals:
    void signalExitThread();
    void signalExitApp();
    void signalExitForwarder();
    void signalExitCupsMonitor();
private:
    StatusServer* m_statusserver;
};

#endif //TIMETHREAD_H