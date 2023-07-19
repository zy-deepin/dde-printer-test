#include "statusserver.h"

StatusServer::StatusServer(const int statusPort, QObject *parent)
    : m_statusPort(statusPort),
      QObject(parent),
      m_bHeart(false),
      m_tcpStatusServer(nullptr) {
    establishServer();
}

StatusServer::~StatusServer() {
    m_tcpStatusClients.clear();
    if (m_tcpStatusServer) {
        m_tcpStatusServer->deleteLater();
    }
}

bool StatusServer::isHeart() {
    return m_bHeart;
}

void StatusServer::establishServer() {
    m_tcpStatusServer = new QTcpServer;
    bool ret = m_tcpStatusServer->listen(QHostAddress::Any, m_statusPort);

    if (!ret) {
        emit serverError();
        qWarning() << "establishServer failed.";
    } else {
        qInfo() << "establishServer success.";
        emit serverEstablished();
    }

    connect(m_tcpStatusServer, &QTcpServer::newConnection, this, &StatusServer::slotNewStatusConnection);
}

void StatusServer::slotNewStatusConnection() {
    // 处理状态信道的新连接
    qInfo() << "new connection.";
    QTcpSocket *currentSocket = m_tcpStatusServer->nextPendingConnection();
    m_tcpStatusClients.push_back(currentSocket);

    // 向客户端发送初始化消息，启动心跳包
    QByteArray sendData("OK");
    
    currentSocket->write(sendData);
    currentSocket->flush();
    currentSocket->waitForBytesWritten();
    
    connect(currentSocket, &QTcpSocket::readyRead, this, &StatusServer::slotStatusReadyRead);
    connect(currentSocket, &QTcpSocket::disconnected, [=, this]() {
        m_tcpStatusClients.removeAll(currentSocket);
        m_bHeart = false;
        qInfo() << "current client socket disconnect.";
    });
}

void StatusServer::slotStatusReadyRead() {
    qDebug() << "heart packet" << this->thread();
    // 状态信道，处理客户端发来的心跳包
    QTcpSocket *currentSocket = (QTcpSocket *) sender();
    QByteArray msgData = currentSocket->readAll();
    qDebug() << "msgData " << msgData;
    if ("heart" == msgData) {
        m_bHeart = true;
    }
}