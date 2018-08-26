QT += quick widgets multimedia
CONFIG += c++14
CONFIG += object_parallel_to_source
CONFIG += qtquickcompiler

TARGET = matrique

include(include/libqmatrixclient/libqmatrixclient.pri)
include(include/SortFilterProxyModel/SortFilterProxyModel.pri)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += src/main.cpp \
    src/controller.cpp \
    src/roomlistmodel.cpp \
    src/imageprovider.cpp \
    src/messageeventmodel.cpp \
    src/imageproviderconnection.cpp \
    src/emojimodel.cpp \
    src/matriqueroom.cpp \
    src/userlistmodel.cpp

RESOURCES += \
    res.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
unix:!mac:isEmpty(PREFIX) {
    message("Install PREFIX not set; using /usr/local. You can change this with 'qmake PREFIX=...'")
    PREFIX = /usr/local
}
unix:target.path = $$PREFIX/bin
win32:target.path = $$PREFIX
!isEmpty(target.path): INSTALLS += target

unix:!mac {
    metainfo.files = $$PWD/org.eu.encom.matrique.appdata.xml
    metainfo.path = $$PREFIX/share/metainfo
    desktop.files = $$PWD/org.eu.encom.matrique.desktop
    desktop.path = $$PREFIX/share/applications
    icons.files = $$PWD/icons/hicolor/*
    icons.path = $$PREFIX/share/icons/hicolor
    INSTALLS += metainfo desktop icons
}

win32 {
    RC_ICONS = asset/img/icon.ico
}

mac {
    ICON = asset/img/icon.icns
}

DISTFILES += \
    ChatForm.qml \
    LoginForm.qml \
    main.qml \
    Home.qml \
    Login.qml \
    ImageStatus.qml \
    ButtonDelegate.qml \
    SideNav.qml \
    RoomListForm.qml \
    RoomDetailForm.qml \
    Room.qml \
    Setting.qml \
    qml/js/md.js

HEADERS += \
    src/controller.h \
    src/roomlistmodel.h \
    src/imageprovider.h \
    src/messageeventmodel.h \
    src/imageproviderconnection.h \
    src/emojimodel.h \
    src/matriqueroom.h \
    src/userlistmodel.h
