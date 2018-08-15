QT += quick widgets
CONFIG += c++14
CONFIG += object_parallel_to_source
CONFIG += qtquickcompiler

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
    src/emojimodel.cpp

RESOURCES += \
    res.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

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
    src/emojimodel.h

unix:!mac {
    metainfo.files = $$PWD/matrique.appdata.xml
    metainfo.path = $$PREFIX/share/metainfo
    desktop.files = $$PWD/matrique.desktop
    desktop.path = $$PREFIX/share/applications
    icons.files = $$PWD/icons/matrique.png
    icons.path = $$PREFIX/share/icons/hicolor/64x64/apps
    INSTALLS += metainfo desktop mime icons
}
