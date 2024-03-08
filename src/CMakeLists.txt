# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

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
    registration.cpp
    neochatconnection.cpp
    neochatconnection.h
    jobs/neochatdeactivateaccountjob.cpp
    jobs/neochatdeactivateaccountjob.h
    jobs/neochatdeletedevicejob.cpp
    jobs/neochatdeletedevicejob.h
    jobs/neochatchangepasswordjob.cpp
    jobs/neochatchangepasswordjob.h
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
    events/serveraclevent.h
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
)

qt_add_qml_module(neochat URI org.kde.neochat NO_PLUGIN
    QML_FILES
        qml/main.qml
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
        qml/RoomWindow.qml
        qml/ExploreRoomsPage.qml
        qml/ManualRoomDialog.qml
        qml/ExplorerDelegate.qml
        qml/InviteUserPage.qml
        qml/ImageEditorPage.qml
        qml/WelcomePage.qml
        qml/General.qml
        qml/RoomSecurity.qml
        qml/PushNotification.qml
        qml/Categories.qml
        qml/Permissions.qml
        qml/NeochatMaximizeComponent.qml
        qml/FancyEffectsContainer.qml
        qml/TypingPane.qml
        qml/QuickSwitcher.qml
        qml/HoverActions.qml
        qml/ChatBar.qml
        qml/AttachmentPane.qml
        qml/ReplyPane.qml
        qml/CompletionMenu.qml
        qml/PieProgressBar.qml
        qml/QuickFormatBar.qml
        qml/RoomData.qml
        qml/ServerData.qml
        qml/EmojiPicker.qml
        qml/TimelineDelegate.qml
        qml/ReplyComponent.qml
        qml/StateDelegate.qml
        qml/MessageDelegate.qml
        qml/Bubble.qml
        qml/SectionDelegate.qml
        qml/ReactionDelegate.qml
        qml/EventDelegate.qml
        qml/ReadMarkerDelegate.qml
        qml/MimeComponent.qml
        qml/StateComponent.qml
        qml/MessageEditComponent.qml
        qml/AvatarFlow.qml
        qml/LoginStep.qml
        qml/Login.qml
        qml/Homeserver.qml
        qml/Username.qml
        qml/RegisterPassword.qml
        qml/Captcha.qml
        qml/Terms.qml
        qml/Email.qml
        qml/Password.qml
        qml/LoginRegister.qml
        qml/Loading.qml
        qml/LoginMethod.qml
        qml/Sso.qml
        qml/UserDetailDialog.qml
        qml/CreateRoomDialog.qml
        qml/EmojiDialog.qml
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
        qml/GlobalMenu.qml
        qml/EditMenu.qml
        qml/MessageDelegateContextMenu.qml
        qml/FileDelegateContextMenu.qml
        qml/MessageSourceSheet.qml
        qml/ReportSheet.qml
        qml/SettingsPage.qml
        qml/ThemeRadioButton.qml
        qml/ColorScheme.qml
        qml/GeneralSettingsPage.qml
        qml/EmoticonsPage.qml
        qml/EmoticonEditorPage.qml
        qml/EmoticonFormCard.qml
        qml/GlobalNotificationsPage.qml
        qml/NotificationRuleItem.qml
        qml/AppearanceSettingsPage.qml
        qml/AccountsPage.qml
        qml/AccountEditorPage.qml
        qml/DevicesPage.qml
        qml/DeviceDelegate.qml
        qml/DevicesCard.qml
        qml/About.qml
        qml/AboutKDE.qml
        qml/SonnetConfigPage.qml
        qml/NetworkProxyPage.qml
        qml/DevtoolsPage.qml
        qml/ConfirmEncryptionDialog.qml
        qml/RemoveSheet.qml
        qml/BanSheet.qml
        qml/EmojiTonesPicker.qml
        qml/EmojiDelegate.qml
        qml/EmojiGrid.qml
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
        qml/Security.qml
        qml/QrCodeMaximizeComponent.qml
        qml/SelectSpacesDialog.qml
        qml/AttachDialog.qml
        qml/NotificationsView.qml
        qml/LoadingDelegate.qml
        qml/TimelineEndDelegate.qml
        qml/SearchPage.qml
        qml/ServerComboBox.qml
        qml/UserSearchPage.qml
        qml/ManualUserDialog.qml
        qml/MessageComponentChooser.qml
        qml/TextComponent.qml
        qml/ImageComponent.qml
        qml/VideoComponent.qml
        qml/AudioComponent.qml
        qml/EncryptedComponent.qml
        qml/FileComponent.qml
        qml/LocationComponent.qml
        qml/LiveLocationComponent.qml
        qml/PollComponent.qml
        qml/LinkPreviewComponent.qml
        qml/LoadComponent.qml
        qml/RecommendedSpaceDialog.qml
        qml/RoomTreeSection.qml
        qml/DelegateContextMenu.qml
        qml/ShareDialog.qml
        qml/FeatureFlagPage.qml
        qml/IgnoredUsersDialog.qml
        qml/AccountData.qml
        qml/StateKeys.qml
        qml/CodeComponent.qml
        qml/QuoteComponent.qml
    RESOURCES
        qml/confetti.png
        qml/glowdot.png
)

if(UNIX)
    qt_target_qml_sources(neochat QML_FILES qml/ShareAction.qml)
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

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    target_compile_definitions(neochat PUBLIC -DHAVE_RUNNER)
    target_compile_definitions(neochat PUBLIC -DHAVE_X11)
    target_sources(neochat PRIVATE runner.cpp)
endif()

target_include_directories(neochat PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models ${CMAKE_CURRENT_SOURCE_DIR}/enums)
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
