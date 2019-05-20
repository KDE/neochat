QT += quick widgets multimedia

unix:!mac {
    QT += dbus
}

mac {
    QT += macextras
}

CONFIG += c++14
CONFIG += object_parallel_to_source
CONFIG += link_pkgconfig

TARGET = spectral

isEmpty(USE_SYSTEM_SORTFILTERPROXYMODEL) {
    USE_SYSTEM_SORTFILTERPROXYMODEL = false
}
isEmpty(USE_SYSTEM_QMATRIXCLIENT) {
    USE_SYSTEM_QMATRIXCLIENT = false
}

$$USE_SYSTEM_QMATRIXCLIENT {
    PKGCONFIG += QMatrixClient
} else {
    message("Falling back to built-in libQMatrixClient.")
    include(include/libQuotient/libqmatrixclient.pri)
}
$$USE_SYSTEM_SORTFILTERPROXYMODEL {
    PKGCONFIG += SortFilterProxyModel
} else {
    message("Falling back to built-in SortFilterProxyModel.")
    include(include/SortFilterProxyModel/SortFilterProxyModel.pri)
}

INCLUDEPATH += include/hoedown
HEADERS += \
    include/hoedown/autolink.h \
    include/hoedown/buffer.h \
    include/hoedown/document.h \
    include/hoedown/escape.h \
    include/hoedown/html.h \
    include/hoedown/stack.h \
    include/hoedown/version.h

SOURCES += \
    include/hoedown/autolink.c \
    include/hoedown/buffer.c \
    include/hoedown/document.c \
    include/hoedown/escape.c \
    include/hoedown/html.c \
    include/hoedown/html_blocks.c \
    include/hoedown/html_smartypants.c \
    include/hoedown/stack.c \
    include/hoedown/version.c

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RESOURCES += res.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH += imports/

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH += imports/

# Default rules for deployment.
unix:!mac:isEmpty(PREFIX) {
    message("Install PREFIX not set; using /usr/local. You can change this with 'qmake PREFIX=...'")
    PREFIX = /usr/local
}
unix:!mac:isEmpty(BINDIR) {
    message("Install BINDIR not set; using PREFIX/bin. You can change this with 'qmake BINDIR=...'")
    BINDIR = $$PREFIX/bin
}
unix:!mac:target.path = $$BINDIR
mac:target.path = $$PREFIX/bin
win32:target.path = $$PREFIX
!isEmpty(target.path): INSTALLS += target

unix:!mac {
    metainfo.files = $$PWD/org.eu.encom.spectral.appdata.xml
    metainfo.path = $$PREFIX/share/metainfo
    desktop.files = $$PWD/org.eu.encom.spectral.desktop
    desktop.path = $$PREFIX/share/applications
    icons.files = $$PWD/icons/hicolor/*
    icons.path = $$PREFIX/share/icons/hicolor
    INSTALLS += metainfo desktop icons
}

win32 {
    RC_ICONS = assets/img/icon.ico
}

mac {
    QMAKE_TARGET_BUNDLE_PREFIX = org.eu.encom

    VERSION = 0.0.0.$$system(git rev-list --count HEAD)
    INFO_PLIST_PATH = $$shell_quote($${OUT_PWD}/$${TARGET}.app/Contents/Info.plist)
    QMAKE_POST_LINK += /usr/libexec/PlistBuddy -c \"Set :CFBundleVersion $${VERSION}\" $${INFO_PLIST_PATH}
    QMAKE_POST_LINK += && /usr/libexec/PlistBuddy -c \"Set :CFBundleShortVersionString $${VERSION}\" $${INFO_PLIST_PATH}

    ICON = assets/img/icon.icns
    QMAKE_INFO_PLIST = macOS/Info.plist
}

HEADERS += \
    src/controller.h \
    src/roomlistmodel.h \
    src/messageeventmodel.h \
    src/emojimodel.h \
    src/spectralroom.h \
    src/userlistmodel.h \
    src/accountlistmodel.h \
    src/spectraluser.h \
    src/notifications/manager.h \
    src/utils.h \
    src/imageclipboard.h \
    src/matriximageprovider.h

SOURCES += src/main.cpp \
    src/controller.cpp \
    src/roomlistmodel.cpp \
    src/messageeventmodel.cpp \
    src/emojimodel.cpp \
    src/spectralroom.cpp \
    src/userlistmodel.cpp \
    src/accountlistmodel.cpp \
    src/spectraluser.cpp \
    src/utils.cpp \
    src/imageclipboard.cpp \
    src/matriximageprovider.cpp

unix:!mac {
    SOURCES += src/notifications/managerlinux.cpp
}

win32 {
    HEADERS += src/notifications/wintoastlib.h
    SOURCES += src/notifications/managerwin.cpp \
        src/notifications/wintoastlib.cpp
}

mac {
    QMAKE_LFLAGS += -framework Foundation -framework Cocoa
    SOURCES += src/notifications/managermac.mm
}
