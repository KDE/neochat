add_executable(neochat
    accountlistmodel.cpp
    controller.cpp
    emojimodel.cpp
    imageclipboard.cpp
    matriximageprovider.cpp
    messageeventmodel.cpp
    roomlistmodel.cpp
    spectralroom.cpp
    spectraluser.cpp
    trayicon.cpp
    userlistmodel.cpp
    publicroomlistmodel.cpp
    userdirectorylistmodel.cpp
    utils.cpp
    main.cpp
    notificationsmanager.cpp
    ../res.qrc
)

target_link_libraries(neochat PRIVATE Qt5::Quick Qt5::Qml Qt5::Gui Qt5::Network Qt5::QuickControls2 KF5::I18n KF5::Kirigami2 KF5::Notifications KF5::ConfigCore Quotient cmark::cmark)

if(ANDROID)
    target_link_libraries(neochat PRIVATE Qt5::Svg OpenSSL::SSL)
    kirigami_package_breeze_icons(ICONS
        "document-send"
    )
else()
    target_link_libraries(neochat PRIVATE Qt5::Widgets ${QTKEYCHAIN_LIBRARIES})
endif()

install(TARGETS neochat ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
