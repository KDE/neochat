# SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(LibNeoChat STATIC
    neochatconnection.cpp
    neochatconnection.h
    neochatroom.cpp
    neochatroom.h
    neochatroommember.cpp
    neochatroommember.h
    chatbarcache.cpp
    chatbarcache.h
    chatdocumenthandler.cpp
    chatdocumenthandler.h
    clipboard.cpp
    clipboard.h
    eventhandler.cpp
    eventhandler.h
    filetransferpseudojob.cpp
    filetransferpseudojob.h
    filetype.cpp
    filetype.h
    linkpreviewer.cpp
    linkpreviewer.h
    messagecomponent.h
    pollhandler.cpp
    pollhandler.h
    spacehierarchycache.cpp
    spacehierarchycache.h
    enums/messagecomponenttype.h
    enums/messagetype.h
    enums/powerlevel.cpp
    enums/powerlevel.h
    enums/pushrule.h
    events/locationbeaconevent.h
    events/pollevent.cpp
    events/pollevent.h
    events/widgetevent.h
    models/actionsmodel.cpp
    models/actionsmodel.h
    models/completionmodel.cpp
    models/completionmodel.h
    models/itinerarymodel.cpp
    models/itinerarymodel.h
    models/messagecontentmodel.cpp
    models/messagecontentmodel.h
    models/reactionmodel.cpp
    models/reactionmodel.h
    models/roomlistmodel.cpp
    models/roomlistmodel.h
    models/threadmodel.cpp
    models/threadmodel.h
    models/threepidmodel.cpp
    models/threepidmodel.h
    utils.cpp
    utils.h
)
generate_export_header(LibNeoChat BASE_NAME libNeoChat)

configure_file(config-neochat.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-neochat.h)

ecm_qt_declare_logging_category(neochat
    HEADER "eventhandler_logging.h"
    IDENTIFIER "EventHandling"
    CATEGORY_NAME "org.kde.neochat.eventhandler"
    DEFAULT_SEVERITY Info
)

target_link_libraries(LibNeoChat PUBLIC
    Qt::Core
    Qt::Qml
    Qt::Quick
    Qt::Multimedia
    KF6::CoreAddons
    KF6::I18n
    KF6::Kirigami
    KF6::SonnetCore
    QuotientQt6
    QCoro::Core
)

if(NOT ANDROID)
    target_link_libraries(LibNeoChat PUBLIC KF6::KIOWidgets KF6::SyntaxHighlighting)
endif()
