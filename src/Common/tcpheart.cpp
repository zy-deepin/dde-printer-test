
#include "tcpheart.h"


TcpHeart::TcpHeart(QObject *parent)
    : QObject(parent) {
    m_heart_timer = new QTimer(this);
    connect(m_heart_timer, &QTimer::timeout, this, &TcpHeart::slotTimeOut);
}

TcpHeart::~TcpHeart() {
    if (m_heart_timer) {
        m_heart_timer->stop();
        m_heart_timer->deleteLater();
    }
}

void TcpHeart::startHeartTimer() {
    m_heart_timer->start(2000);
}

void TcpHeart::slotTimeOut() {
    emit sigHeartReq();
}