#-------------------------------------------------
#
# Project created by QtCreator 2010-10-31T08:46:00
#
#-------------------------------------------------

TARGET = SynapseCore
TEMPLATE = lib

DEFINES += SYNAPSECORE_BUILD
QMAKE_CXXFLAGS += -Wall

SOURCES += syplugin.cpp

HEADERS += syplugin.h \
    syglobal.h

INCLUDEPATH += ../../EksCore ../../Shift ../../alter2 ../../alter2/plugins/ShiftAlter

DESTDIR += ../../bin

LIBS += -L../../bin -lshift -lEksCore -lalter -lShiftAlter -lShiftGraphicsCore