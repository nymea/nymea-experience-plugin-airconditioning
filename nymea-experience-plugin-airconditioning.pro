TEMPLATE = lib
TARGET = $$qtLibraryTarget(nymea_experiencepluginairconditioning)

greaterThan(QT_MAJOR_VERSION, 5) {
    message("Building using Qt6 support")
    CONFIG *= c++17
    QMAKE_LFLAGS *= -std=c++17
    QMAKE_CXXFLAGS *= -std=c++17
} else {
    message("Building using Qt5 support")
    CONFIG *= c++11
    QMAKE_LFLAGS *= -std=c++11
    QMAKE_CXXFLAGS *= -std=c++11
    DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x050F00
}

CONFIG += plugin link_pkgconfig
PKGCONFIG += nymea

QT -= gui
QT += network sql

HEADERS += experiencepluginairconditioning.h \
    airconditioningjsonhandler.h \
    airconditioningmanager.h \
    notifications.h \
    temperatureschedule.h \
    thermostat.h \
    zoneinfo.h

SOURCES += experiencepluginairconditioning.cpp \
    airconditioningjsonhandler.cpp \
    airconditioningmanager.cpp \
    notifications.cpp \
    temperatureschedule.cpp \
    thermostat.cpp \
    zoneinfo.cpp


target.path = $$[QT_INSTALL_LIBS]/nymea/experiences/
INSTALLS += target

# Install translation files
TRANSLATIONS *= $$files($${_PRO_FILE_PWD_}/translations/*ts, true)
lupdate.depends = FORCE
lupdate.depends += qmake_all
lupdate.commands = lupdate -recursive -no-obsolete $${_PRO_FILE_PWD_}/experience.pro
QMAKE_EXTRA_TARGETS += lupdate

# make lrelease to build .qm from .ts
lrelease.depends = FORCE
lrelease.commands += lrelease $$files($$_PRO_FILE_PWD_/translations/*.ts, true);
QMAKE_EXTRA_TARGETS += lrelease

translations.depends += lrelease
translations.path = /usr/share/nymea/translations
translations.files = $$[QT_SOURCE_TREE]/translations/*.qm
INSTALLS += translations

