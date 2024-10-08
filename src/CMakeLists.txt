# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE AND NOT NEOCHAT_FLATPAK AND NOT NEOCHAT_APPIMAGE)
    add_subdirectory(purpose)
endif()

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
    models/spacechildrenmodel.cpp
    models/spacechildrenmodel.h
    models/spacechildsortfiltermodel.cpp
    models/spacechildsortfiltermodel.h
    models/spacetreeitem.cpp
    models/spacetreeitem.h
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
    models/roomtreemodel.cpp
    models/roomtreemodel.h
    chatdocumenthandler.cpp
    chatdocumenthandler.h
    models/devicesmodel.cpp
    models/devicesmodel.h
    models/devicesproxymodel.cpp
    filetype.cpp
    filetype.h
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
    utils.cpp
    registration.cpp
    neochatconnection.cpp
    neochatconnection.h
    jobs/neochatdeactivateaccountjob.cpp
    jobs/neochatdeactivateaccountjob.h
    jobs/neochatdeletedevicejob.cpp
    jobs/neochatdeletedevicejob.h
    jobs/neochatchangepasswordjob.cpp
    jobs/neochatchangepasswordjob.h
    jobs/neochatgetcommonroomsjob.cpp
    jobs/neochatgetcommonroomsjob.h
    mediasizehelper.cpp
    mediasizehelper.h
    eventhandler.cpp
    enums/delegatetype.h
    roomlastmessageprovider.cpp
    roomlastmessageprovider.h
    chatbarcache.cpp
    chatbarcache.h
    colorschemer.cpp
    colorschemer.h
    models/notificationsmodel.cpp
    models/notificationsmodel.h
    models/timelinemodel.cpp
    models/timelinemodel.h
    enums/pushrule.h
    models/itinerarymodel.cpp
    models/itinerarymodel.h
    proxycontroller.cpp
    proxycontroller.h
    models/linemodel.cpp
    models/linemodel.h
    events/locationbeaconevent.h
    events/widgetevent.h
    enums/messagecomponenttype.h
    models/messagecontentmodel.cpp
    models/messagecontentmodel.h
    enums/neochatroomtype.h
    models/sortfilterroomtreemodel.cpp
    models/sortfilterroomtreemodel.h
    mediamanager.cpp
    mediamanager.h
    models/statekeysmodel.cpp
    models/statekeysmodel.h
    sharehandler.cpp
    sharehandler.h
    models/roomtreeitem.cpp
    models/roomtreeitem.h
    foreigntypes.h
    models/threepidmodel.cpp
    models/threepidmodel.h
    threepidaddhelper.cpp
    threepidaddhelper.h
    jobs/neochatadd3pidjob.cpp
    jobs/neochatadd3pidjob.h
    identityserverhelper.cpp
    identityserverhelper.h
    enums/powerlevel.cpp
    enums/powerlevel.h
    models/permissionsmodel.cpp
    models/permissionsmodel.h
    threepidbindhelper.cpp
    threepidbindhelper.h
    models/readmarkermodel.cpp
    models/readmarkermodel.h
    neochatroommember.cpp
    neochatroommember.h
    models/threadmodel.cpp
    models/threadmodel.h
    enums/messagetype.h
    messagecomponent.h
)

set_source_files_properties(qml/OsmLocationPlugin.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

ecm_add_qml_module(neochat URI org.kde.neochat GENERATE_PLUGIN_SOURCE
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat
    QML_FILES
        qml/Main.qml
        qml/AccountMenu.qml
        qml/ExploreComponent.qml
        qml/ExploreComponentMobile.qml
        qml/ContextMenu.qml
        qml/CollapsedRoomDelegate.qml
        qml/RoomDelegate.qml
        qml/RoomListPage.qml
        qml/SpaceListContextMenu.qml
        qml/UserInfo.qml
        qml/UserInfoDesktop.qml
        qml/RoomPage.qml
        qml/ExploreRoomsPage.qml
        qml/ManualRoomDialog.qml
        qml/ExplorerDelegate.qml
        qml/InviteUserPage.qml
        qml/ImageEditorPage.qml
        qml/NeochatMaximizeComponent.qml
        qml/TypingPane.qml
        qml/QuickSwitcher.qml
        qml/HoverActions.qml
        qml/AttachmentPane.qml
        qml/QuickFormatBar.qml
        qml/UserDetailDialog.qml
        qml/CreateRoomDialog.qml
        qml/OpenFileDialog.qml
        qml/KeyVerificationDialog.qml
        qml/ConfirmLogoutDialog.qml
        qml/PowerLevelDialog.qml
        qml/Message.qml
        qml/EmojiItem.qml
        qml/EmojiRow.qml
        qml/EmojiSas.qml
        qml/ConfirmDeactivateAccountDialog.qml
        qml/VerificationCanceled.qml
        qml/MessageDelegateContextMenu.qml
        qml/FileDelegateContextMenu.qml
        qml/MessageSourceSheet.qml
        qml/ConfirmEncryptionDialog.qml
        qml/RoomSearchPage.qml
        qml/LocationChooser.qml
        qml/TimelineView.qml
        qml/InvitationView.qml
        qml/AvatarTabButton.qml
        qml/SpaceDrawer.qml
        qml/OsmLocationPlugin.qml
        qml/FullScreenMap.qml
        qml/LocationsPage.qml
        qml/LocationMapItem.qml
        qml/RoomDrawer.qml
        qml/RoomDrawerPage.qml
        qml/DirectChatDrawerHeader.qml
        qml/GroupChatDrawerHeader.qml
        qml/RoomInformation.qml
        qml/RoomMedia.qml
        qml/ChooseRoomDialog.qml
        qml/SpaceHomePage.qml
        qml/SpaceHierarchyDelegate.qml
        qml/RemoveChildDialog.qml
        qml/SelectParentDialog.qml
        qml/QrCodeMaximizeComponent.qml
        qml/SelectSpacesDialog.qml
        qml/NotificationsView.qml
        qml/SearchPage.qml
        qml/ServerComboBox.qml
        qml/UserSearchPage.qml
        qml/ManualUserDialog.qml
        qml/RecommendedSpaceDialog.qml
        qml/RoomTreeSection.qml
        qml/DelegateContextMenu.qml
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
    DEPENDENCIES
        QtCore
        QtQuick
    IMPORTS
        org.kde.neochat.timeline
        org.kde.neochat.settings
        org.kde.neochat.devtools
        org.kde.neochat.login
        org.kde.neochat.chatbar
)

add_subdirectory(settings)
add_subdirectory(timeline)
add_subdirectory(devtools)
add_subdirectory(login)
add_subdirectory(chatbar)

if(NOT ANDROID AND NOT WIN32)
    qt_target_qml_sources(neochat QML_FILES
        qml/ShareAction.qml
        qml/GlobalMenu.qml
        qml/EditMenu.qml
    )
else()
    set_source_files_properties(qml/ShareActionStub.qml PROPERTIES
        QT_RESOURCE_ALIAS qml/ShareAction.qml
    )
    qt_target_qml_sources(neochat QML_FILES qml/ShareActionStub.qml)
endif()


configure_file(config-neochat.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-neochat.h)

if(WIN32)
    set_target_properties(neochat PROPERTIES OUTPUT_NAME "neochatlib")
endif()

ecm_qt_declare_logging_category(neochat
    HEADER "messageeventmodel_logging.h"
    IDENTIFIER "MessageEvent"
    CATEGORY_NAME "org.kde.neochat.messageeventmodel"
    DESCRIPTION "Neochat: messageeventmodel"
    DEFAULT_SEVERITY Info
    EXPORT NEOCHAT
)

ecm_qt_declare_logging_category(neochat
    HEADER "publicroomlist_logging.h"
    IDENTIFIER "PublicRoomList"
    CATEGORY_NAME "org.kde.neochat.publicroomlistmodel"
    DESCRIPTION "Neochat: publicroomlistmodel"
    DEFAULT_SEVERITY Info
    EXPORT NEOCHAT
)

ecm_qt_declare_logging_category(neochat
    HEADER "eventhandler_logging.h"
    IDENTIFIER "EventHandling"
    CATEGORY_NAME "org.kde.neochat.eventhandler"
    DEFAULT_SEVERITY Info
)

ecm_qt_declare_logging_category(neochat
    HEADER "chatdocumenthandler_logging.h"
    IDENTIFIER "ChatDocumentHandling"
    CATEGORY_NAME "org.kde.neochat.chatdocumenthandler"
    DEFAULT_SEVERITY Info
)

add_executable(neochat-app
    main.cpp
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
    if (NOT WIN32 AND NOT APPLE)
        target_sources(neochat PRIVATE trayicon_sni.cpp trayicon_sni.h)
        target_link_libraries(neochat PRIVATE KF6::StatusNotifierItem)
    else()
        target_sources(neochat PRIVATE trayicon.cpp trayicon.h)
    endif()
    target_link_libraries(neochat PUBLIC KF6::WindowSystem ICU::uc)
    target_compile_definitions(neochat PUBLIC -DHAVE_WINDOWSYSTEM)
    target_compile_definitions(neochat PUBLIC -DHAVE_ICU)
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

target_include_directories(neochat PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models ${CMAKE_CURRENT_SOURCE_DIR}/enums)
target_link_libraries(neochat PRIVATE settingsplugin timelineplugin devtoolsplugin loginplugin chatbarplugin)
target_link_libraries(neochat PUBLIC
    Qt::Core
    Qt::Quick
    Qt::Qml
    Qt::Gui
    Qt::Multimedia
    Qt::Network
    Qt::QuickControls2
    KF6::I18n
    KF6::Kirigami
    KF6::Notifications
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::SonnetCore
    KF6::ColorScheme
    KF6::ItemModels
    QuotientQt6
    cmark::cmark
    QCoro::Core
    QCoro::Network
)

if (TARGET KF6::Crash)
    target_link_libraries(neochat PUBLIC KF6::Crash)
endif()

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
        "security-low"
        "security-low-symbolic"
        "kde"
        "list-remove-symbolic"
        "edit-delete"
    )
    ecm_add_android_apk(neochat-app ANDROID_DIR ${CMAKE_SOURCE_DIR}/android)
else()
    target_link_libraries(neochat PUBLIC Qt::Widgets KF6::KIOWidgets KF6::SyntaxHighlighting)
    install(FILES neochat.notifyrc DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})
endif()

if(NOT ANDROID)
    set_target_properties(neochat-app PROPERTIES OUTPUT_NAME "neochat")
endif()

if(TARGET KF6::DBusAddons AND NOT WIN32)
    target_link_libraries(neochat PUBLIC KF6::DBusAddons)
    target_compile_definitions(neochat PUBLIC -DHAVE_KDBUSADDONS)
endif()

if (TARGET KF6::KIOWidgets)
    target_compile_definitions(neochat PUBLIC -DHAVE_KIO)
endif()

if (TARGET KUnifiedPush)
    target_compile_definitions(neochat PUBLIC -DHAVE_KUNIFIEDPUSH)
    target_link_libraries(neochat PUBLIC KUnifiedPush)

    if (NOT ANDROID)
        configure_file(org.kde.neochat.service.in ${CMAKE_CURRENT_BINARY_DIR}/org.kde.neochat.service)
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.neochat.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
    endif()
endif()

install(TARGETS neochat-app ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    install(FILES plasma-runner-neochat.desktop DESTINATION ${KDE_INSTALL_DATAROOTDIR}/krunner/dbusplugins)
endif()
