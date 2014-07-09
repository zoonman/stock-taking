# -------------------------------------------------
# Project created by QtCreator 2010-02-05T22:05:28
# -------------------------------------------------
QT += core sql printsupport
TARGET = stock-taking
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    dialogmysqlconfig.cpp \
    dialogprintpreview.cpp \
    reportdialog.cpp \
    shippingdialog.cpp
HEADERS += mainwindow.h \
    dialogmysqlconfig.h \
    dialogprintpreview.h \
    reportdialog.h \
    shippingdialog.h
FORMS += mainwindow.ui \
    dialogmysqlconfig.ui \
    dialogprintpreview.ui \
    reportdialog.ui \
    shippingdialog.ui
RESOURCES += resources.qrc
RC_FILE = myapp.rc
