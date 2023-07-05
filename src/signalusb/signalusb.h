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
#ifndef SIGNALUSB_H
#define SIGNALUSB_H

#include <QThread>
#include <QMutex>

#include <libusb-1.0/libusb.h>


class SignalUSB : public QThread
{
    Q_OBJECT
public:
    SignalUSB(QObject *parent = nullptr);
    ~SignalUSB() override;

    /*用于注册libusb回调*/
    static int LIBUSB_CALL static_usb_arrived_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *userdata);

    int  usb_arrived_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event);

protected:
    void run() override;

private:
    bool needExit;

private:

    void callDdePrinterHelper();
};

#endif // SIGNALUSB_H
