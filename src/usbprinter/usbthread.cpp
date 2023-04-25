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
#include "usbthread.h"
#include "cupsconnectionfactory.h"
#include "cupsattrnames.h"
#include "qtconvert.h"
#include "addprinter.h"
#include "zdrivermanager.h"
#include "printerservice.h"


#include <DApplication>
#include <DNotifySender>

#include <QDBusPendingReply>
#include <QDBusConnection>
#include <QProcess>

#include <map>
#include <string>

#include <cups/cups.h>

using namespace std;
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

static bool isUSBPrinterDevice(const struct libusb_interface_descriptor *interface)
{
    if (!interface)
        return false;
    if ((interface->bInterfaceClass == LIBUSB_CLASS_PRINTER) && (interface->bInterfaceSubClass == 1)) {
        return true;
    }
    return false;
}

static bool isUSBPrinterDevice(const struct libusb_interface *interface)
{
    bool ret = false;
    if (!interface)
        return ret;
    int i;
    for (i = 0; i < interface->num_altsetting; i++) {
        ret = isUSBPrinterDevice(&interface->altsetting[i]);
        if (ret)
            break;
    }

    return ret;
}

static bool isUSBPrinterDevice(const struct libusb_config_descriptor *config)
{
    bool ret = false;
    if (!config)
        return ret;
    int i;
    for (i = 0; i < config->bNumInterfaces; i++) {
        ret = isUSBPrinterDevice(&config->interface[i]);
        if (ret)
            break;
    }

    return ret;
}


/*判断打印机是否已经添加过*/
static bool isArrivedUSBPrinterAdded(const map<string, string> &infoMap)
{

    if (infoMap.count("Manufacturer") == 0 || infoMap.count("Product") == 0 || infoMap.count("SerialNumber") == 0)
        {
            qInfo() << "can't meet condition.";
            return true;
        }
    /*同一台惠普打印机可能会存在两个uri，需要都查找出来，作为打印机是否添加判断添加，避免重复添加*/
    vector<string> inSechemes{"usb", "hp"};
    vector<string> exSechemes;
    map<string, map<string, string> > devsMap;
    std::unique_ptr<Connection> conPtr;
    try {
        conPtr = CupsConnectionFactory::createConnectionBySettings();
        if (conPtr)
            devsMap = conPtr->getDevices(&exSechemes, &inSechemes, 0, CUPS_TIMEOUT_DEFAULT);
    } catch (const std::exception &ex) {
        qWarning() << "Got execpt: " << QString::fromUtf8(ex.what());
        return true;

    }
    QStringList uriList;
    map<string, string> devices;
    /*从usb后端发现的设备中查找对应序列号的设备*/
    for (auto itmap = devsMap.begin(); itmap != devsMap.end(); itmap++) {
        QString uri = QString::fromStdString(itmap->first);
        if (uri.contains(QString::fromStdString(infoMap.at("SerialNumber"))) && (uri.startsWith("usb:") || uri.startsWith("hp:"))) {
            uriList << uri;
            devices = itmap->second;
        }
    }
    if (uriList.isEmpty() || devices.size() == 0) {
        qWarning() << QString("device not found from cups,product:%1");
        return true;
    }
    /*从cups返回的已经添加的打印机中查找是否存在对应的uri*/
    map<string, map<string, string>> printersMap;
    try {
        printersMap = conPtr->getPrinters();
        for (auto iter = printersMap.begin(); iter != printersMap.end(); iter++) {
            map<string, string> mapProperty = iter->second;
            if (mapProperty.count(CUPS_DEV_URI) == 0)
                continue;
            QString strValue = attrValueToQString(mapProperty[CUPS_DEV_URI]);

            if (uriList.contains(strValue)) {
                return true;
            }
        }
    } catch (const std::runtime_error &e) {
        qWarning() << "Got execpt: " << QString::fromUtf8(e.what());
        return true;
    }

    return false;
}



static map<string, string>  getInfomationFromDescription(libusb_device_handle *pHandle, const libusb_device_descriptor &desc)
{
    map<string, string> infoMap;
    if (!pHandle)
        {
            
            return infoMap;
        }
    printf("pHandle=%p\n",pHandle);
    unsigned char ustring[256];
    memset(ustring, 0, sizeof(ustring));
    int ret = 0;
    if (desc.iManufacturer) {
        qInfo() << "111 desc.iManufacturer:" << desc.iManufacturer;

        ret = libusb_get_string_descriptor_ascii(pHandle, desc.iManufacturer, ustring, sizeof(ustring));
        qInfo() << "ret1:" << ret;
        if (ret > 0) {
            qInfo() << QString("Manufacturer:%1").arg((char *)ustring);
            infoMap.insert(make_pair<string, string>("Manufacturer", (char *)ustring));
        }

    }
    memset(ustring, 0, sizeof(ustring));
    if (desc.iProduct) {
        qInfo() << "222 desc.iProduct:" << desc.iProduct;
        ret = libusb_get_string_descriptor_ascii(pHandle, desc.iProduct, ustring, sizeof(ustring));
        qInfo() << "ret2:" << ret;
        if (ret > 0) {
            qInfo() << QString("Product:%1").arg((char *)ustring);
            infoMap.insert(make_pair<string, string>("Product", (char *)ustring));
        }

    }
    memset(ustring, 0, sizeof(ustring));
    if (desc.iSerialNumber) {
        qInfo() << "333 desc.iSerialNumber" << desc.iSerialNumber;
        ret = libusb_get_string_descriptor_ascii(pHandle, desc.iSerialNumber, ustring, sizeof(ustring));
        qInfo() << "ret3:" << ret;
        if (ret > 0) {
            infoMap.insert(make_pair<string, string>("SerialNumber", (char *)ustring));
        }

    }
    return infoMap;
}


USBThread::USBThread(QObject *parent)
    : QThread(parent)
    , needExit(false)
{

}

USBThread::~USBThread()
{

}

void USBThread::run()
{
    qInfo() << "USB monitor running...";
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
    qInfo() << "USB monitor exit";
}

void USBThread::addPrinter(unsigned int vid, unsigned int pid)
{
    QProcess process;
    QString cmd = "dde-printer-helper";
    QStringList args;
    args << "-u" << "addprinter" << to_string(vid).c_str() << to_string(pid).c_str();
    if (!process.startDetached(cmd, args)) {
        qWarning() << QString("showJobsWindow failed because %1").arg(process.errorString());
    }
}

int LIBUSB_CALL USBThread::static_usb_arrived_callback(struct libusb_context *ctx, struct libusb_device *dev,
                                                       libusb_hotplug_event event, void *userdata)
{
    printf("static_usb_arrived_callback\n");
    if (userdata)
        return reinterpret_cast<USBThread *>(userdata)->usb_arrived_callback(ctx, dev, event);
    else {
        qWarning() << "userdata is null";
        return -1;
    }
}

int USBThread::usb_arrived_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event)
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
    
    libusb_device_handle *pHandle = nullptr;
    struct libusb_device_descriptor desc;
    int ret = libusb_get_device_descriptor(dev, &desc);
    if (ret < 0) {
        qWarning() << "failed to get device descriptor";
        return (dev) ? 0 : -1;
    }
    
    qInfo() << QString("Device vendor:%1 product:%2 iSerialNumber:%3").arg(desc.idVendor).arg(desc.idProduct).arg(desc.iSerialNumber);

    if (!pHandle)
    {
        libusb_open(dev, &pHandle);
        qInfo() << "USB open!!";
    }

    // 1.判断是否是打印机驱动
    bool isUSBPrinter = false;
    for (uint8_t i = 0; i < desc.bNumConfigurations; i++) {
        struct libusb_config_descriptor *config = nullptr;

        ret = libusb_get_config_descriptor(dev, i, &config);
        if (LIBUSB_SUCCESS != ret) {
            qWarning() << "Couldn't retrieve descriptors";
            continue;
        }

        isUSBPrinter = isUSBPrinterDevice(config);

        libusb_free_config_descriptor(config);
        if (isUSBPrinter)
            break;
    }

    if (isUSBPrinter) {
        addPrinter(desc.idVendor,desc.idProduct);
    } else {
        qInfo() << "the driver is not a printer driver.";
        goto end;
    }

#if 0
    // 2.判断该打印机驱动是否添加过
    bool isAdded = true;
    if (isUSBPrinter) {
        map<string, string> infoMap = getInfomationFromDescription(pHandle, desc);
        isAdded = isArrivedUSBPrinterAdded(infoMap);
    } else {
        qInfo() << "the driver is not a printer driver.";
        goto end;
    }

    // 3.调用dde-printer-helper进程执行添加打印机的操作
    if (!isAdded) {
        qInfo() << "begin to add printer driver.";
        addPrinter(desc.idVendor,desc.idProduct);
    } else {
        qInfo() << "The printer has been added.";
    }

#endif
end:
    if (pHandle)
    {
        libusb_close(pHandle);
        qInfo() << "USB close!!";
    }
    return (dev) ? 0 : -1;
}

