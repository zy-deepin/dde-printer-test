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

#ifndef SIGNALCUPS_H
#define SIGNALCUPS_H

#include <QDBusMessage>
#include <QThread>

#include <string>
#include <vector>

class SignalCups : public QThread
{
    Q_OBJECT

public:
    SignalCups(QObject *parent = nullptr);
    ~SignalCups() override;
    void stop();
    bool initWatcher();

protected:

    void run() override;
    int doWork();
    void callDdePrinterHelper();

protected slots:
    void spoolerEvent(QDBusMessage);

};


#endif //SIGNALCUPS_H
