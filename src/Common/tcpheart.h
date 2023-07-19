#ifndef TCPHEART_H
#define TCPHEART_H

#include <QObject>
#include <QTimer>

class TcpHeart : public QObject {
    Q_OBJECT
public:
    explicit TcpHeart(QObject* parent = 0);
    ~TcpHeart();

public:
    void startHeartTimer();  // 启动定时器

signals:
    void sigHeartReq();  // 发送心跳包信号

private slots:
    void slotTimeOut();  // 定时事件

private:
    QTimer* m_heart_timer;
};

#endif //TCPHEART_H
