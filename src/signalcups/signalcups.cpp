/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
 *
 * Author:     Wei xie <xiewei@deepin.com>
 *
 * Maintainer: Wei xie  <xiewei@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "signalcups.h"

#include <DApplication>
#include <DNotifySender>

#include <QDebug>
#include <QProcess>
#include <QDBusConnection>
#include <QDBusPendingReply>

#include <algorithm>
#include <regex>

#include <pwd.h>
DWIDGET_USE_NAMESPACE

DCORE_USE_NAMESPACE

SignalCups::SignalCups(QObject *parent)
    : QThread(parent)
{
    
}

SignalCups::~SignalCups()
{
    stop();
}

void SignalCups::run()
{
    qInfo() << "Task SignalCups running...";
    int iRet = 0;

    iRet = doWork();

    qInfo() << "Task SignalCups finished " << iRet;

}

void SignalCups::callDdePrinterHelper()
{
    QProcess process;
    QString cmd = "dde-printer-helper";
    QStringList args;
    if (!process.startDetached(cmd, args)) {
        qWarning() << QString("callDdePrinterHelper failed because %1").arg(process.errorString());
    }
}

int SignalCups::doWork()
{
    while (1) {
        callDdePrinterHelper();
        break;
    }

    return 0;
}

void SignalCups::stop()
{
    if (this->isRunning()) {
        this->quit();
        this->wait();
    }
}

bool SignalCups::initWatcher()
{
    QDBusConnection conn = QDBusConnection::systemBus();
    /*关联系统的打印队列，当线程退出的时候，如果有新的打印队列，重新唤醒线程*/
    bool success = conn.connect("", "/com/redhat/PrinterSpooler", "com.redhat.PrinterSpooler", "", this, SLOT(spoolerEvent(QDBusMessage)));

    start();

    if (!success) {
        qInfo() << "failed to connect spooler dbus";
    }
    return success;
}

void SignalCups::spoolerEvent(QDBusMessage msg)
{
    QList<QVariant> args = msg.arguments();
    qDebug() << args;

    if (!isRunning()) {
        start();
    }
}