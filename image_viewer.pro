#-------------------------------------------------
#
# Project created by QtCreator 2015-12-19T14:14:51
#
#-------------------------------------------------

QT       += core gui webkit network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = image_viewer
TEMPLATE = app
INCLUDEPATH += "C:\Users\Vlad\Downloads\opencv_3.0\release\install\include"
LIBS += "C:\Users\Vlad\Downloads\opencv_3.0\release\bin\libopencv_core310.dll"
LIBS += "C:\Users\Vlad\Downloads\opencv_3.0\release\bin\libopencv_highgui310.dll"
LIBS += "C:\Users\Vlad\Downloads\opencv_3.0\release\bin\libopencv_imgcodecs310.dll"
SOURCES += main.cpp\
        mainwindow.cpp \
    custom_w/qimagewidjet.cpp \
    geocode_data_manager.cpp \
    C:\Users\Vlad\Documents\qt-json-master\json.cpp \
    button/qslideswitch.cpp\
    custom_w/taglist.cpp

HEADERS  += mainwindow.h \
    geocode_data_manager.h \
    custom_w/qimagewidjet.h \
C:\Users\Vlad\Documents\qt-json-master\json.h \
    button/qslideswitch_p.h \
    button/qslideswitch.h\
    custom_w/taglist.h

FORMS    += mainwindow.ui


OTHER_FILES += \
    google_maps.html \
    C:\Users\Vlad\Documents\qt-json-master\AUTHORS                                       \
    C:\Users\Vlad\Documents\qt-json-master\LICENSE                                       \
    C:\Users\Vlad\Documents\qt-json-master\README.md                                     \
    C:\Users\Vlad\Documents\qt-google-maps-master\test.sqlite

RESOURCES += \
    resource.qrc
