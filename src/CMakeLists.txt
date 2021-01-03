add_executable(neochat
    accountlistmodel.cpp
    controller.cpp
    emojimodel.cpp
    clipboard.cpp
    matriximageprovider.cpp
    messageeventmodel.cpp
    roomlistmodel.cpp
    neochatroom.cpp
    neochatuser.cpp
    userlistmodel.cpp
    publicroomlistmodel.cpp
    userdirectorylistmodel.cpp
    utils.cpp
    main.cpp
    notificationsmanager.cpp
    sortfilterroomlistmodel.cpp
    chatdocumenthandler.cpp
    devicesmodel.cpp
    filetypesingleton.cpp
    ../res.qrc
)

if(NOT ANDROID)
    target_sources(neochat PRIVATE trayicon.cpp)
endif()

target_include_directories(neochat PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(neochat PRIVATE Qt5::Quick Qt5::Qml Qt5::Gui Qt5::Network Qt5::QuickControls2 KF5::I18n KF5::Kirigami2 KF5::Notifications KF5::ConfigCore KF5::ConfigGui KF5::CoreAddons Quotient cmark::cmark)
kconfig_add_kcfg_files(neochat GENERATE_MOC neochatconfig.kcfgc)

if(NEOCHAT_FLATPAK)
    target_compile_definitions(neochat PRIVATE NEOCHAT_FLATPAK)
endif()

if(ANDROID)
    target_link_libraries(neochat PRIVATE Qt5::Svg OpenSSL::SSL)
    kirigami_package_breeze_icons(ICONS
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
    )
else()
    target_link_libraries(neochat PRIVATE Qt5::Widgets KF5::DBusAddons ${QTKEYCHAIN_LIBRARIES})
endif()

install(TARGETS neochat ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
