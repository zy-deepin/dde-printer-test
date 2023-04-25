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
#ifndef USBTHREADHELPER_H
#define USBTHREADHELPER_H

#include <QThread>
#include <QMutex>

#include "zdevicemanager.h"

#include <libusb-1.0/libusb.h>


class USBThreadHelper : public QObject
{
    Q_OBJECT
public:
    USBThreadHelper(QObject *parent = nullptr);
    ~USBThreadHelper() override;

    void addPrinter(unsigned int vid, unsigned int pid);
    

private:
    int getUsbDevice(unsigned int vid, unsigned int pid);
    void getDriver();
    void nextConfiguration();

private:
    TDeviceInfo m_deviceInfo;
    QString m_configingPrinterName;
    QMap<uint, QString> m_pendingNotificationsMap;
    libusb_device *m_currentUSBDevice;
    QList<libusb_device *> m_usbDeviceList;


private slots:
    void processArrivedUSBDevice();
    bool addArrivedUSBPrinter();
    void notificationActionInvoked(uint id, const QString &msg);
    void addingJobFinished(int status);
    
signals:
    void newUSBDeviceArrived();
    /*通知前端当前正在配置的打印机状态变化*/
    //void deviceStatusChanged(const QString &defaultPrinterName, int status);
};

#endif // USBTHREADHELPER_H
