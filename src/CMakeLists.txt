# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <fella@posteo.de>
# SPDX-License-Identifier: BSD-2-Clause

add_library(neochat STATIC
    controller.cpp
    actionshandler.cpp
    emojimodel.cpp
    customemojimodel.cpp
    customemojimodel+network.cpp
    clipboard.cpp
    matriximageprovider.cpp
    messageeventmodel.cpp
    messagefiltermodel.cpp
    roomlistmodel.cpp
    sortfilterspacelistmodel.cpp
    spacehierarchycache.cpp
    roommanager.cpp
    neochatroom.cpp
    neochatuser.cpp
    userlistmodel.cpp
    publicroomlistmodel.cpp
    userdirectorylistmodel.cpp
    utils.cpp
    notificationsmanager.cpp
    sortfilterroomlistmodel.cpp
    chatdocumenthandler.cpp
    devicesmodel.cpp
    filetypesingleton.cpp
    login.cpp
    stickerevent.cpp
    webshortcutmodel.cpp
    blurhash.cpp
    blurhashimageprovider.cpp
    joinrulesevent.cpp
    collapsestateproxymodel.cpp
    urlhelper.cpp
    windowcontroller.cpp
    linkpreviewer.cpp
    completionmodel.cpp
    completionproxymodel.cpp
    actionsmodel.cpp
)

add_executable(neochat-app
    main.cpp
    ../res.qrc
)

target_include_directories(neochat-app PRIVATE ${CMAKE_BINARY_DIR})

target_link_libraries(neochat-app PRIVATE
    neochat
)

if(Quotient_VERSION_MINOR GREATER 6)
    target_compile_definitions(neochat PUBLIC QUOTIENT_07)
else()
    target_sources(neochat PRIVATE neochataccountregistry.cpp)
endif()

ecm_add_app_icon(NEOCHAT_ICON ICONS ${CMAKE_SOURCE_DIR}/128-logo.png)

target_sources(neochat PRIVATE ${NEOCHAT_ICON})

if(NOT ANDROID)
    target_sources(neochat PRIVATE colorschemer.cpp)
    if (NOT WIN32 AND NOT APPLE)
        target_sources(neochat PRIVATE trayicon_sni.cpp)
    else()
        target_sources(neochat PRIVATE trayicon.cpp)
    endif()
    target_link_libraries(neochat PUBLIC KF5::ConfigWidgets KF5::WindowSystem)
    target_compile_definitions(neochat PUBLIC -DHAVE_COLORSCHEME)
    target_compile_definitions(neochat PUBLIC -DHAVE_WINDOWSYSTEM)
endif()

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    target_sources(neochat-app PRIVATE ../res_desktop.qrc)
    target_compile_definitions(neochat PUBLIC -DHAVE_RUNNER)
    target_sources(neochat PRIVATE runner.cpp)
else()
    target_sources(neochat-app PRIVATE ../res_android.qrc)
endif()

target_include_directories(neochat PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(neochat PUBLIC Qt::Core Qt::Quick Qt::Qml Qt::Gui Qt::Multimedia Qt::Network Qt::QuickControls2 KF5::I18n KF5::Kirigami2 KF5::Notifications KF5::ConfigCore KF5::ConfigGui KF5::CoreAddons KF5::SonnetCore Quotient cmark::cmark ${QTKEYCHAIN_LIBRARIES})
if(TARGET QCoro5::Coro)
    target_link_libraries(neochat PUBLIC QCoro5::Coro)
else()
    target_link_libraries(neochat PUBLIC QCoro::QCoro)
endif()
kconfig_add_kcfg_files(neochat GENERATE_MOC neochatconfig.kcfgc)

if(NEOCHAT_FLATPAK)
    target_compile_definitions(neochat PUBLIC NEOCHAT_FLATPAK)
endif()

if(ANDROID)
    target_sources(neochat PRIVATE notifyrc.qrc)
    target_link_libraries(neochat PRIVATE Qt::Svg OpenSSL::SSL)
    target_sources(neochat-app PRIVATE notifyrc.qrc)
    target_link_libraries(neochat PUBLIC Qt::Svg OpenSSL::SSL)
    kirigami_package_breeze_icons(ICONS
        "arrow-down"
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
        "edit-copy"
        "gtk-quit"
        "compass"
        "network-connect"
        "list-remove-user"
        "org.kde.neochat"
        "preferences-system-users"
        "preferences-desktop-theme-global"
        "notifications"
        "zoom-in"
        "zoom-out"
        "image-rotate-left-symbolic"
        "image-rotate-right-symbolic"
    )
else()
    target_link_libraries(neochat PUBLIC Qt::Widgets KF5::KIOWidgets)
    install(FILES neochat.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
endif()

if(NOT ANDROID)
    set_target_properties(neochat-app PROPERTIES OUTPUT_NAME "neochat")
endif()

if(TARGET KF5::DBusAddons)
    target_link_libraries(neochat PUBLIC KF5::DBusAddons)
    target_compile_definitions(neochat PUBLIC -DHAVE_KDBUSADDONS)
endif()

if (TARGET KF5::KIOWidgets)
    target_compile_definitions(neochat PUBLIC -DHAVE_KIO)
endif()

install(TARGETS neochat-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    install(FILES plasma-runner-neochat.desktop DESTINATION ${KDE_INSTALL_DATAROOTDIR}/krunner/dbusplugins)
endif()

