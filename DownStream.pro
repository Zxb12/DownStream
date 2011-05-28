#-------------------------------------------------
#
# Project created by QtCreator 2011-04-10T18:07:14
#
#-------------------------------------------------

QT       += core gui network

TARGET = DownStream
TEMPLATE = app


SOURCES += src/main.cpp\
    src/fenprincipale.cpp \
    src/downloadhandler.cpp \
    src/auth.cpp \
    src/linkextractor.cpp \
    src/download.cpp \
    src/fenoptions.cpp \
    src/vitessetransfert.cpp \
    src/versioncheck.cpp \
    src/paquet.cpp \
    src/log.cpp

HEADERS  += src/fenprincipale.h \
    src/downloadhandler.h \
    src/auth.h \
    src/linkextractor.h \
    src/enums.h \
    src/download.h \
    src/fenoptions.h \
    src/vitessetransfert.h \
    src/versioncheck.h \
    src/paquet.h \
    src/log.h

FORMS    += src/fenprincipale.ui \
    src/fenoptions.ui

RESOURCES += \
    res/ressource.qrc

OTHER_FILES += \
    TODO.txt \
    Changelog.txt
