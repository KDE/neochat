# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

configure_file(qml/Page/RoomList/RoomDelegate.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/Page/RoomList/RoomDelegate.qml)
configure_file(qml/Component/QuickSwitcher.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/Component/QuickSwitcher.qml)
configure_file(qml/Dialog/PowerLevelDialog.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/Dialog/PowerLevelDialog.qml)
configure_file(qml/Component/Timeline/AudioDelegate.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/Component/Timeline/AudioDelegate.qml)
configure_file(qml/Component/Timeline/VideoDelegate.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/Component/Timeline/VideoDelegate.qml)
configure_file(qml/Component/Timeline/OsmLocationPlugin.qml ${CMAKE_CURRENT_BINARY_DIR}/qml/Component/Timeline/OsmLocationPlugin.qml)

configure_file(res.qrc ${CMAKE_CURRENT_SOURCE_DIR}/res.generated.qrc)

add_library(neochat STATIC
    controller.cpp
    controller.h
    actionshandler.cpp
    actionshandler.h
    models/emojimodel.cpp
    models/emojimodel.h
    emojitones.cpp
    emojitones.h
    models/customemojimodel.cpp
    models/customemojimodel.h
    clipboard.cpp
    clipboard.h
    matriximageprovider.cpp
    matriximageprovider.h
    models/messageeventmodel.cpp
    models/messageeventmodel.h
    models/messagefiltermodel.cpp
    models/messagefiltermodel.h
    models/roomlistmodel.cpp
    models/roomlistmodel.h
    models/sortfilterspacelistmodel.cpp
    models/sortfilterspacelistmodel.h
    models/accountemoticonmodel.cpp
    models/accountemoticonmodel.h
    spacehierarchycache.cpp
    spacehierarchycache.h
    roommanager.cpp
    roommanager.h
    neochatroom.cpp
    neochatroom.h
    models/userlistmodel.cpp
    models/userlistmodel.h
    models/userfiltermodel.cpp
    models/userfiltermodel.h
    models/publicroomlistmodel.cpp
    models/publicroomlistmodel.h
    models/userdirectorylistmodel.cpp
    models/userdirectorylistmodel.h
    models/pushrulemodel.cpp
    models/pushrulemodel.h
    models/emoticonfiltermodel.cpp
    models/emoticonfiltermodel.h
    notificationsmanager.cpp
    notificationsmanager.h
    models/sortfilterroomlistmodel.cpp
    models/sortfilterroomlistmodel.h
    chatdocumenthandler.cpp
    chatdocumenthandler.h
    models/devicesmodel.cpp
    models/devicesmodel.h
    models/devicesproxymodel.cpp
    filetypesingleton.cpp
    filetypesingleton.h
    login.cpp
    login.h
    models/webshortcutmodel.cpp
    models/webshortcutmodel.h
    blurhash.cpp
    blurhash.h
    blurhashimageprovider.cpp
    blurhashimageprovider.h
    models/mediamessagefiltermodel.cpp
    models/mediamessagefiltermodel.h
    urlhelper.cpp
    urlhelper.h
    windowcontroller.cpp
    windowcontroller.h
    linkpreviewer.cpp
    linkpreviewer.h
    models/completionmodel.cpp
    models/completionmodel.h
    models/completionproxymodel.cpp
    models/completionproxymodel.h
    models/actionsmodel.cpp
    models/actionsmodel.h
    models/serverlistmodel.cpp
    models/serverlistmodel.h
    models/statemodel.cpp
    models/statemodel.h
    models/statefiltermodel.cpp
    models/statefiltermodel.h
    filetransferpseudojob.cpp
    filetransferpseudojob.h
    models/searchmodel.cpp
    models/searchmodel.h
    texthandler.cpp
    texthandler.h
    logger.cpp
    logger.h
    models/stickermodel.cpp
    models/stickermodel.h
    models/imagepacksmodel.cpp
    models/imagepacksmodel.h
    events/imagepackevent.cpp
    events/imagepackevent.h
    events/joinrulesevent.cpp
    events/joinrulesevent.h
    models/reactionmodel.cpp
    models/reactionmodel.h
    delegatesizehelper.cpp
    delegatesizehelper.h
    models/livelocationsmodel.cpp
    models/livelocationsmodel.h
    models/locationsmodel.cpp
    models/locationsmodel.h
    locationhelper.cpp
    locationhelper.h
    events/pollevent.cpp
    pollhandler.cpp
    utils.h
    registration.cpp
)

ecm_qt_declare_logging_category(neochat
    HEADER "messageeventmodel_logging.h"
    IDENTIFIER "MessageEvent"
    CATEGORY_NAME "org.kde.neochat.messageeventmodel"
    DESCRIPTION "Neochat: messageeventmodel"
    DEFAULT_SEVERITY Info
    EXPORT NEOCHAT
)

add_executable(neochat-app
    main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/res.generated.qrc
)

if(TARGET Qt::WebView)
    target_link_libraries(neochat-app PUBLIC Qt::WebView)
    target_compile_definitions(neochat-app PUBLIC -DHAVE_WEBVIEW)
endif()

target_include_directories(neochat-app PRIVATE ${CMAKE_BINARY_DIR})

target_link_libraries(neochat-app PRIVATE
    neochat
)

ecm_add_app_icon(NEOCHAT_ICON ICONS ${CMAKE_SOURCE_DIR}/128-logo.png)

target_sources(neochat-app PRIVATE ${NEOCHAT_ICON})

if(NOT ANDROID)
    target_sources(neochat PRIVATE colorschemer.cpp colorschemer.h)
    if (NOT WIN32 AND NOT APPLE)
        target_sources(neochat PRIVATE trayicon_sni.cpp trayicon_sni.h)
        if(QT_MAJOR_VERSION STREQUAL "6")
            target_link_libraries(neochat PRIVATE KF6::StatusNotifierItem)
        endif()
    else()
        target_sources(neochat PRIVATE trayicon.cpp trayicon.h)
    endif()
    target_link_libraries(neochat PUBLIC KF${QT_MAJOR_VERSION}::ConfigWidgets KF${QT_MAJOR_VERSION}::WindowSystem)
    target_compile_definitions(neochat PUBLIC -DHAVE_COLORSCHEME)
    target_compile_definitions(neochat PUBLIC -DHAVE_WINDOWSYSTEM)
endif()

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    target_sources(neochat-app PRIVATE res_desktop.qrc)
    target_compile_definitions(neochat PUBLIC -DHAVE_RUNNER)
    target_compile_definitions(neochat PUBLIC -DHAVE_X11)
    target_sources(neochat PRIVATE runner.cpp)
else()
    target_sources(neochat-app PRIVATE res_android.qrc)
endif()

target_include_directories(neochat PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(neochat PUBLIC Qt::Core Qt::Quick Qt::Qml Qt::Gui Qt::Multimedia Qt::Network Qt::QuickControls2 KF${QT_MAJOR_VERSION}::I18n KF${QT_MAJOR_VERSION}::Kirigami2 KF${QT_MAJOR_VERSION}::Notifications KF${QT_MAJOR_VERSION}::ConfigCore KF${QT_MAJOR_VERSION}::ConfigGui KF${QT_MAJOR_VERSION}::CoreAddons KF${QT_MAJOR_VERSION}::SonnetCore  KF${QT_MAJOR_VERSION}::ItemModels Quotient${QUOTIENT_SUFFIX} cmark::cmark QCoro::Core)

kconfig_add_kcfg_files(neochat GENERATE_MOC neochatconfig.kcfgc)

if(NEOCHAT_FLATPAK)
    target_compile_definitions(neochat PUBLIC NEOCHAT_FLATPAK)
endif()

if(ANDROID)
    target_sources(neochat PRIVATE notifyrc.qrc)
    target_link_libraries(neochat PRIVATE Qt::Svg OpenSSL::SSL)
    if(SQLite3_FOUND)
        target_link_libraries(neochat-app PRIVATE SQLite::SQLite3)
    endif()
    target_sources(neochat-app PRIVATE notifyrc.qrc)
    target_link_libraries(neochat PUBLIC Qt::Svg OpenSSL::SSL)
    kirigami_package_breeze_icons(ICONS
        "arrow-down"
        "arrow-up"
        "checkmark"
        "help-about"
        "im-user"
        "im-invisible-user"
        "im-kick-user"
        "mail-attachment"
        "dialog-cancel"
        "preferences-desktop-emoticons"
        "document-open"
        "document-save"
        "document-send"
        "dialog-close"
        "edit-delete-remove"
        "code-context"
        "document-edit"
        "list-user-add"
        "list-add-user"
        "user-others"
        "media-playback-pause"
        "media-playback-start"
        "media-playback-stop"
        "go-previous"
        "go-up"
        "go-down"
        "list-add"
        "irc-join-channel"
        "settings-configure"
        "configure"
        "rating"
        "rating-unrated"
        "search"
        "mail-replied-symbolic"
        "edit-clear"
        "edit-copy"
        "gtk-quit"
        "compass"
        "computer"
        "network-connect"
        "list-remove-user"
        "org.kde.neochat"
        "preferences-system-users"
        "preferences-desktop-theme-global"
        "notifications"
        "notifications-disabled"
        "audio-volume-high"
        "audio-volume-muted"
        "draw-highlight"
        "zoom-in"
        "zoom-out"
        "image-rotate-left-symbolic"
        "image-rotate-right-symbolic"
        "channel-secure-symbolic"
        "download"
        "smiley"
        "tools-check-spelling"
        "username-copy"
        "system-switch-user"
        "bookmark-new"
        "bookmark-remove"
        "favorite"
        "window-new"
        "globe"
        "visibility"
        "home"
        "preferences-desktop-notification"
        "computer-symbolic"
        "gps"
    )
else()
    target_link_libraries(neochat PUBLIC Qt::Widgets KF${QT_MAJOR_VERSION}::KIOWidgets)
    install(FILES neochat.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
endif()

if(NOT ANDROID)
    set_target_properties(neochat-app PROPERTIES OUTPUT_NAME "neochat")
endif()

if(TARGET KF${QT_MAJOR_VERSION}::DBusAddons)
    target_link_libraries(neochat PUBLIC KF${QT_MAJOR_VERSION}::DBusAddons)
    target_compile_definitions(neochat PUBLIC -DHAVE_KDBUSADDONS)
endif()

if (TARGET KF${QT_MAJOR_VERSION}::KIOWidgets)
    target_compile_definitions(neochat PUBLIC -DHAVE_KIO)
endif()

install(TARGETS neochat-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    install(FILES plasma-runner-neochat.desktop DESTINATION ${KDE_INSTALL_DATAROOTDIR}/krunner/dbusplugins)
endif()

