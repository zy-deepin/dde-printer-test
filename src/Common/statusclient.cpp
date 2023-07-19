#include "statusclient.h"

StatusClient::StatusClient(const QString host, const int port, QObject *parent)
    : m_host(host),
      m_port(port),
      QObject(parent) {
    connectedToServer();
}

StatusClient::~StatusClient() {
    slotHeartBad();
    if (m_statusSocket) {
        m_statusSocket->deleteLater();
    }
    if (m_heart) {
        m_heart->deleteLater();
    }
}

void StatusClient::connectedToServer() {
    qInfo() << "connectedToServer " << m_host << m_port;
    m_statusSocket = new QTcpSocket;

    connect(m_statusSocket, &QTcpSocket::readyRead, this, &StatusClient::slotStatusReadyRead);
    connect(m_statusSocket, &QTcpSocket::disconnected, this, &StatusClient::signalDisconnectedToServer);

    m_statusSocket->connectToHost(m_host, m_port);

    if (!m_statusSocket->waitForConnected()) {
        qWarning() << "connect to server failed.";
        emit signalDisconnectedToServer();
    }
}

void StatusClient::slotStatusReadyRead() {
    qInfo() << "status ready read.";
    // 收到服务端发来的客户端状态
    QByteArray data;
    data = m_statusSocket->readAll();

    QString dataStr = QString::fromUtf8(data);
    qInfo() << "dataStr: " << dataStr;
    //服务器返回"OK"，开启心跳包检测
    if ("OK" == dataStr) { 
        m_heart = new TcpHeart;
        // 开始心跳检测
        m_heart->startHeartTimer();
        connect(m_heart, &TcpHeart::sigHeartReq, this, &StatusClient::slotWriteHeartSocket);// 发送心跳包
    }
}

void StatusClient::slotWriteHeartSocket() {
    QByteArray data("heart");
    // 状态信道，向服务端发送心跳包
    qDebug() << this->thread() << "send a heart packet.";

    bool ret = m_statusSocket->write(data);
    m_statusSocket->waitForBytesWritten();

    if (!ret) {
        qWarning() << "send a heart packet failed.";
        emit signalDisconnectedToServer();
    }
}

void StatusClient::slotHeartBad() {
    // 断线处理
    qInfo() << "heart break.";
    m_statusSocket->disconnectFromHost();
    emit signalDisconnectedToServer();
}