#ifndef STATUSSERVER_H
#define STATUSSERVER_H

#include <QDebug>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class StatusServer : public QObject {
    Q_OBJECT
public:
    explicit StatusServer(const int statusPort, QObject *parent = nullptr);
    ~StatusServer();

    bool isHeart();

signals:
    void serverEstablished();
    void serverError();
public slots:
    void establishServer();
private slots:
    void slotNewStatusConnection();// 状态信道链接
    void slotStatusReadyRead();

private:
    // 状态信道
    QTcpServer *m_tcpStatusServer;
    QList<QTcpSocket *> m_tcpStatusClients;

private:
    int m_statusPort;
    bool m_bHeart; //是否心跳
};

#endif //STATUSSERVER_H