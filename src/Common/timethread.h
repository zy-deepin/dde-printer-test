#ifndef TIMETHREAD_H
#define TIMETHREAD_H

#include <QThread>
#include <QTime>
#include <pthread.h>

#define IDLEEXIT 1000 * 60 * 5

class TimeThread : public QThread
{
    Q_OBJECT
public:
    TimeThread();
    ~TimeThread();
protected:
    void run() override;
    void stop();
signals:
    void signalExitThread();
    void signalExitApp();
    void signalExitForwarder();
    void signalExitCupsMonitor();
};

#endif //TIMETHREAD_H