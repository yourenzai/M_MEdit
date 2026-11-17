QT += widgets testlib
CONFIG += c++17 testcase
TEMPLATE = app

SOURCES += test_mainwindow.cpp \
           ../mainwindow.cpp

HEADERS += ../mainwindow.h \
           ../M_MEdit_ui.h

INCLUDEPATH += ..
