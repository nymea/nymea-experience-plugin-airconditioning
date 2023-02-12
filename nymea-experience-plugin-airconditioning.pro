TEMPLATE = lib
TARGET = $$qtLibraryTarget(nymea_experiencepluginairconditioning)

CONFIG += plugin link_pkgconfig c++11
PKGCONFIG += nymea

QT -= gui
QT += network sql

include(../config.pri)

HEADERS += experiencepluginairconditioning.h \
    airconditioningjsonhandler.h \
    airconditioningmanager.h \
    temperatureschedule.h \
    thermostat.h \
    zoneinfo.h

SOURCES += experiencepluginairconditioning.cpp \
    airconditioningjsonhandler.cpp \
    airconditioningmanager.cpp \
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

