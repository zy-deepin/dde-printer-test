/*
 * Copyright (C) 2019 ~ 2020 Uniontech Software Co., Ltd.
 *
 * Author:     liurui <liurui@uniontech.com>
 *
 * Maintainer: liurui <liurui@uniontech.com>
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
#include "signalusb.h"

#include <QProcess>
#include <QDebug>
#include <DApplication>
#include <DNotifySender>

#include <map>
#include <string>

using namespace std;
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

bool IsProcessExist(const QString &processName)
{
    QProcess process;
    process.start("tasklist");
    process.waitForFinished();
 
    QByteArray result = process.readAllStandardOutput();
    QString str = result;
    if(str.contains(processName))
        return true;
    else
        return false;
}

SignalUSB::SignalUSB(QObject *parent)
    : QThread(parent)
    , needExit(false)
{

}

SignalUSB::~SignalUSB()
{

}

void SignalUSB::run()
{
    qInfo() << "SignalUSB monitor running...";
    libusb_hotplug_callback_handle usb_arrived_handle;
    libusb_context *ctx;
    int rc = 0;
    do {

        rc = libusb_init(&ctx);
        if (rc < 0) {
            return;
        }
        rc = libusb_hotplug_register_callback(ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED,
                                              LIBUSB_HOTPLUG_NO_FLAGS, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
                                              LIBUSB_HOTPLUG_MATCH_ANY, static_usb_arrived_callback, this, &usb_arrived_handle);
        if (LIBUSB_SUCCESS != rc) {
            qWarning() << "Error to register usb arrived callback";
            break;
        }

        while (!needExit) {
            libusb_handle_events_completed(ctx, nullptr);
            sleep(1);
        }

        libusb_hotplug_deregister_callback(ctx, usb_arrived_handle);

    } while (false);
    libusb_exit(ctx);
    qInfo() << "SignalUSB monitor exit";
}

void SignalUSB::callDdePrinterHelper()
{
    QProcess process;
    QString cmd = "dde-printer-helper";
    QStringList args;
    if (!process.startDetached(cmd, args)) {
        qWarning() << QString("callDdePrinterHelper failed because %1").arg(process.errorString());
    }
}

int LIBUSB_CALL SignalUSB::static_usb_arrived_callback(struct libusb_context *ctx, struct libusb_device *dev,
                                                       libusb_hotplug_event event, void *userdata)
{
    printf("static_usb_arrived_callback\n");
    if (userdata)
        return reinterpret_cast<SignalUSB *>(userdata)->usb_arrived_callback(ctx, dev, event);
    else {
        qWarning() << "userdata is null";
        return -1;
    }
}

int SignalUSB::usb_arrived_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event)
{
    /*
     * 1.先判断设备类型是否是打印机 bInterfaceClass =7 bInterfaceSubClass=1
     * 2.获取serial
     * 3.调用cups http api 获取 usb类型打印机
     * 4.从列表中找到和serial对应的打印机
     * 5.判断该打印机是否已经被添加过（uri）
     * 6.自动添加打印机并给出系统通知
    */
    /*
     * NOTE:回调函数结束后才能开始IO操作，不然会返回LIBUSB_ERROR_BUSY错误状态
     * 考虑到多个设备同时插入的情况，需要做一个队列排队添加打印机。存在跨线程访问数据的情况，使用锁同步。
     * 当m_currentUSBDevice为空时触发添加流程，添加完m_currentUSBDevice置为空，通过信号触发新的添加流程
    */
    Q_UNUSED(ctx)
    Q_UNUSED(event)

    qInfo() << "USB arrived!!";
    callDdePrinterHelper();
    return (dev) ? 0 : -1;
}

