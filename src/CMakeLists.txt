# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

add_library(neochat STATIC
    controller.cpp
    actionshandler.cpp
    models/emojimodel.cpp
    emojitones.cpp
    models/customemojimodel.cpp
    clipboard.cpp
    matriximageprovider.cpp
    models/messageeventmodel.cpp
    models/messagefiltermodel.cpp
    models/roomlistmodel.cpp
    models/sortfilterspacelistmodel.cpp
    models/accountstickermodel.cpp
    spacehierarchycache.cpp
    roommanager.cpp
    neochatroom.cpp
    neochatuser.cpp
    models/userlistmodel.cpp
    models/userfiltermodel.cpp
    models/publicroomlistmodel.cpp
    models/userdirectorylistmodel.cpp
    models/keywordnotificationrulemodel.cpp
    models/emoticonfiltermodel.cpp
    notificationsmanager.cpp
    models/sortfilterroomlistmodel.cpp
    chatdocumenthandler.cpp
    models/devicesmodel.cpp
    filetypesingleton.cpp
    login.cpp
    models/webshortcutmodel.cpp
    blurhash.cpp
    blurhashimageprovider.cpp
    models/collapsestateproxymodel.cpp
    urlhelper.cpp
    windowcontroller.cpp
    linkpreviewer.cpp
    models/completionmodel.cpp
    models/completionproxymodel.cpp
    models/actionsmodel.cpp
    models/serverlistmodel.cpp
    models/statemodel.cpp
    models/statefiltermodel.cpp
    filetransferpseudojob.cpp
    models/searchmodel.cpp
    texthandler.cpp
    logger.cpp
    models/stickermodel.cpp
    models/imagepacksmodel.cpp
    events/imagepackevent.cpp
    events/joinrulesevent.cpp
    events/stickerevent.cpp
)

ecm_qt_declare_logging_category(neochat
    HEADER "messageeventmodel_logging.h"
    IDENTIFIER "MessageEvent"
    CATEGORY_NAME "org.kde.neochat.messageeventmodel"
    DEFAULT_SEVERITY Info
)

add_executable(neochat-app
    main.cpp
    res.qrc
)

target_include_directories(neochat-app PRIVATE ${CMAKE_BINARY_DIR})

target_link_libraries(neochat-app PRIVATE
    neochat
)

if(Quotient${QUOTIENT_SUFFIX}_VERSION_MINOR GREATER 6)
    target_compile_definitions(neochat PUBLIC QUOTIENT_07)
    target_sources(neochat PRIVATE events/pollevent.cpp pollhandler.cpp)
else()
    target_compile_definitions(neochat PUBLIC QUOTIENT_VERSION=\"${Quotient${QUOTIENT_SUFFIX}_VERSION}\")
    target_sources(neochat PRIVATE neochataccountregistry.cpp)
endif()

ecm_add_app_icon(NEOCHAT_ICON ICONS ${CMAKE_SOURCE_DIR}/128-logo.png)

target_sources(neochat-app PRIVATE ${NEOCHAT_ICON})

if(NOT ANDROID)
    target_sources(neochat PRIVATE colorschemer.cpp)
    if (NOT WIN32 AND NOT APPLE)
        target_sources(neochat PRIVATE trayicon_sni.cpp)
    else()
        target_sources(neochat PRIVATE trayicon.cpp)
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
target_link_libraries(neochat PUBLIC Qt::Core Qt::Quick Qt::Qml Qt::Gui Qt::Multimedia Qt::Network Qt::QuickControls2 KF${QT_MAJOR_VERSION}::I18n KF${QT_MAJOR_VERSION}::Kirigami2 KF${QT_MAJOR_VERSION}::Notifications KF${QT_MAJOR_VERSION}::ConfigCore KF${QT_MAJOR_VERSION}::ConfigGui KF${QT_MAJOR_VERSION}::CoreAddons KF${QT_MAJOR_VERSION}::SonnetCore  KF${QT_MAJOR_VERSION}::ItemModels Quotient${QUOTIENT_SUFFIX} cmark::cmark ${QTKEYCHAIN_LIBRARIES} QCoro::Core)
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

