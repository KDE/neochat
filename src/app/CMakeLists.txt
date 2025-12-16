# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(neochat STATIC
    controller.cpp
    controller.h
    roommanager.cpp
    roommanager.h
    models/userdirectorylistmodel.cpp
    models/userdirectorylistmodel.h
    notificationsmanager.cpp
    notificationsmanager.h
    blurhash.cpp
    blurhash.h
    blurhashimageprovider.cpp
    blurhashimageprovider.h
    windowcontroller.cpp
    windowcontroller.h
    models/serverlistmodel.cpp
    models/serverlistmodel.h
    models/notificationsmodel.cpp
    models/notificationsmodel.h
    proxycontroller.cpp
    proxycontroller.h
    mediamanager.cpp
    mediamanager.h
    sharehandler.cpp
    sharehandler.h
    foreigntypes.h
    identityserverhelper.cpp
    identityserverhelper.h
    models/commonroomsmodel.cpp
    models/commonroomsmodel.h
    texttospeechhelper.h
    texttospeechhelper.cpp
)

set_source_files_properties(qml/OsmLocationPlugin.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

if(ANDROID OR WIN32)
    set_source_files_properties(qml/ShareActionStub.qml PROPERTIES
        QT_QML_SOURCE_TYPENAME ShareAction
    )
    set_source_files_properties(qml/GlobalMenuStub.qml PROPERTIES
            QT_QML_SOURCE_TYPENAME GlobalMenu
    )
else()
    set(EXTRA_IMPORTS org.kde.purpose)
endif()

ecm_add_qml_module(neochat URI org.kde.neochat GENERATE_PLUGIN_SOURCE
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat
    QML_FILES
        qml/Main.qml
        qml/AccountMenu.qml
        qml/CollapsedRoomDelegate.qml
        qml/RoomPage.qml
        qml/ManualRoomDialog.qml
        qml/ExplorerDelegate.qml
        qml/NeochatMaximizeComponent.qml
        qml/QuickSwitcher.qml
        qml/AttachmentPane.qml
        qml/QuickFormatBar.qml
        qml/UserDetailDialog.qml
        qml/OpenFileDialog.qml
        qml/KeyVerificationDialog.qml
        qml/ConfirmLogoutDialog.qml
        qml/VerificationMessage.qml
        qml/EmojiItem.qml
        qml/EmojiRow.qml
        qml/EmojiSas.qml
        qml/VerificationCanceled.qml
        qml/MessageSourceSheet.qml
        qml/LocationChooser.qml
        qml/InvitationView.qml
        qml/AvatarTabButton.qml
        qml/OsmLocationPlugin.qml
        qml/FullScreenMap.qml
        qml/ChooseRoomDialog.qml
        qml/QrCodeMaximizeComponent.qml
        qml/NotificationsView.qml
        qml/ServerComboBox.qml
        qml/UserSearchPage.qml
        qml/ManualUserDialog.qml
        qml/RecommendedSpaceDialog.qml
        qml/ShareDialog.qml
        qml/UnlockSSSSDialog.qml
        qml/QrScannerPage.qml
        qml/JoinRoomDialog.qml
        qml/ConfirmUrlDialog.qml
        qml/AccountSwitchDialog.qml
        qml/ConfirmLeaveDialog.qml
        qml/CodeMaximizeComponent.qml
        qml/EditStateDialog.qml
        qml/ConsentDialog.qml
        qml/AskDirectChatConfirmation.qml
        qml/HoverLinkIndicator.qml
        qml/AvatarNotification.qml
        qml/ReasonDialog.qml
        qml/NewPollDialog.qml
        qml/UserMenu.qml
        qml/MeetingDialog.qml
    DEPENDENCIES
        QtCore
        QtQuick
        io.github.quotient_im.libquotient
    IMPORTS
        org.kde.neochat.libneochat
        org.kde.neochat.rooms
        org.kde.neochat.roominfo
        org.kde.neochat.messagecontent
        org.kde.neochat.timeline
        org.kde.neochat.spaces
        org.kde.neochat.settings
        org.kde.neochat.devtools
        org.kde.neochat.login
        org.kde.neochat.chatbar
        org.kde.config
        org.kde.syntaxhighlighting
        ${EXTRA_IMPORTS}
)

if(NOT ANDROID AND NOT WIN32)
    qt_target_qml_sources(neochat QML_FILES
        qml/ShareAction.qml
        qml/GlobalMenu.qml
    )
else()
    qt_target_qml_sources(neochat QML_FILES
            qml/ShareActionStub.qml
            qml/GlobalMenuStub.qml
    )
endif()

if(WIN32)
    set_target_properties(neochat PROPERTIES OUTPUT_NAME "neochatlib")
endif()

qt_add_executable(neochat-app
    main.cpp
)

if(ANDROID)
    set_target_properties(neochat-app PROPERTIES
        OUTPUT_NAME "neochat-app"
        PREFIX "lib"
    )
endif()

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
    if (NOT WIN32 AND NOT APPLE)
        target_sources(neochat PRIVATE trayicon_sni.cpp trayicon_sni.h)
        target_link_libraries(neochat PRIVATE KF6::StatusNotifierItem)
    else()
        target_sources(neochat PRIVATE trayicon.cpp trayicon.h)
    endif()
    target_link_libraries(neochat PUBLIC KF6::WindowSystem)
    target_compile_definitions(neochat PUBLIC -DHAVE_WINDOWSYSTEM)
endif()

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE AND NOT HAIKU)
    target_compile_definitions(neochat PUBLIC -DHAVE_RUNNER)
    target_compile_definitions(neochat PUBLIC -DHAVE_X11=1)
    target_sources(neochat PRIVATE runner.cpp)

    if (TARGET KUnifiedPush)
        target_sources(neochat PRIVATE fakerunner.cpp)
    endif()
else()
    target_compile_definitions(neochat PUBLIC -DHAVE_X11=0)
endif()

target_include_directories(neochat PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(neochat PRIVATE neochatplugin Loginplugin Roomsplugin RoomInfoplugin MessageContentplugin Timelineplugin Spacesplugin Chatbarplugin Settingsplugin Devtoolsplugin)
target_link_libraries(neochat PUBLIC
    LibNeoChat
    Timeline
    Settings
    Qt::Core
    Qt::Quick
    Qt::Qml
    Qt::Gui
    Qt::Multimedia
    Qt::Network
    Qt::QuickControls2
    Qt::TextToSpeech
    KF6::I18n
    KF6::Kirigami
    KF6::Notifications
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::SonnetCore
    KF6::ItemModels
    KF6::I18nQml
    KirigamiApp
    QuotientQt6
    Login
    Rooms
    MessageContent
    Spaces
)

kconfig_target_kcfg_file(neochat FILE neochatconfig.kcfg CLASS_NAME NeoChatConfig MUTATORS GENERATE_PROPERTIES DEFAULT_VALUE_GETTERS PARENT_IN_CONSTRUCTOR SINGLETON GENERATE_MOC QML_REGISTRATION)

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
        "arrow-down-symbolic"
        "arrow-up-symbolic"
        "arrow-up-double-symbolic"
        "arrow-left-symbolic"
        "arrow-right-symbolic"
        "checkmark"
        "help-about"
        "im-user"
        "im-invisible-user"
        "im-kick-user"
        "mail-attachment"
        "dialog-cancel"
        "preferences-desktop-emoticons"
        "preferences-security"
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
        "org.kde.neochat.tray"
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
        "system-users-symbolic"
        "map-flat"
        "documentinfo"
        "view-list-details"
        "go-previous"
        "mail-forward-symbolic"
        "dialog-warning-symbolic"
        "object-rotate-left"
        "object-rotate-right"
        "add-subtitle"
        "security-high"
        "security-low"
        "security-low-symbolic"
        "kde"
        "list-remove-symbolic"
        "edit-delete"
        "user-home-symbolic"
        "pin-symbolic"
        "kt-restore-defaults-symbolic"
        "user-symbolic"
        "mark-location-symbolic"
        "amarok_playcount"

        ${KIRIGAMI_ADDONS_ICONS}
    )
    ecm_add_android_apk(neochat-app ANDROID_DIR ${CMAKE_SOURCE_DIR}/android)
else()
    target_link_libraries(neochat PUBLIC Qt::Widgets KF6::KIOWidgets)
    install(FILES neochat.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
endif()

if(NOT ANDROID)
    set_target_properties(neochat-app PROPERTIES OUTPUT_NAME "neochat")
endif()

if(TARGET KF6::DBusAddons AND NOT WIN32)
    target_link_libraries(neochat PUBLIC KF6::DBusAddons)
    target_compile_definitions(neochat PUBLIC -DHAVE_KDBUSADDONS)
endif()

if (TARGET KUnifiedPush)
    if (NOT ANDROID)
        configure_file(org.kde.neochat.service.in ${CMAKE_CURRENT_BINARY_DIR}/org.kde.neochat.service)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.neochat.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
    endif()
endif()

install(TARGETS neochat-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    # krunner plugin must be the same as the app id for flatpak to export it
    install(FILES plasma-runner-neochat.desktop DESTINATION ${KDE_INSTALL_DATAROOTDIR}/krunner/dbusplugins RENAME org.kde.neochat.desktop)
endif()

if (APPLE)
    set_target_properties(neochat-app PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.neochat")
    set_target_properties(neochat-app PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "NeoChat")
    set_target_properties(neochat-app PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING ${RELEASE_SERVICE_VERSION})
    set_target_properties(neochat-app PROPERTIES MACOSX_BUNDLE_BUNDLE_VERSION ${RELEASE_SERVICE_VERSION})
endif ()
