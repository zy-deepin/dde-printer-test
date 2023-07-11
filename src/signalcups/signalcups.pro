QT += core dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = signalcups
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui

SOURCES += \
        main.cpp \ 
    signalcups.cpp 

#RESOURCES +=         resources.qrc

QMAKE_CFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -fstack-protector-strong -D_FORTITY_SOURCE=1 -fPIC
QMAKE_CXXFLAGS += -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow -fstack-protector-strong -D_FORTITY_SOURCE=1 -fPIC
QMAKE_LFLAGS += -z noexecstack -pie -z lazy

unix:!macx:{
LIBS += -lusb-1.0
}

HEADERS += \
    signalcups.h 

DISTFILES +=
linux {
isEmpty(PREFIX){
    PREFIX = /usr
}

target.path = $${PREFIX}/bin

watch.path = /etc/xdg/autostart
watch.files = $${PWD}/platform/linux/watch/dde-printer-watch.desktop

trans.path =  $${PREFIX}/share/dde-printer-helper/translations
trans.files = $${PWD}/translations/*.qm

INSTALLS += target watch trans
}

TRANSLATIONS  +=  translations/dde-printer-helper.ts

#CONFIG(release, debug|release) {
#    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
#}

