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
    manager.cpp
    ../res.qrc
)

target_link_libraries(neochat Qt5::Widgets Qt5::Quick Qt5::Qml Qt5::Gui Qt5::Network Qt5::QuickControls2 KF5::Kirigami2 Quotient cmark::cmark ${QTKEYCHAIN_LIBRARIES})

install(TARGETS neochat ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})
