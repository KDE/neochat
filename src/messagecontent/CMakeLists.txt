# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(MessageContent STATIC)
ecm_add_qml_module(MessageContent GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.messagecontent
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/messagecontent
    QML_FILES
        BaseMessageComponentChooser.qml
        MessageComponentChooser.qml
        ReplyMessageComponentChooser.qml
        ThreadBodyMessageComponentChooser.qml
        AuthorComponent.qml
        AudioComponent.qml
        CodeComponent.qml
        EncryptedComponent.qml
        FetchButtonComponent.qml
        FileComponent.qml
        ImageComponent.qml
        ItineraryComponent.qml
        ItineraryReservationComponent.qml
        JourneySectionStopDelegateLineSegment.qml
        TransportIcon.qml
        FoodReservationComponent.qml
        TrainReservationComponent.qml
        FlightReservationComponent.qml
        HotelReservationComponent.qml
        LinkPreviewComponent.qml
        LinkPreviewLoadComponent.qml
        LiveLocationComponent.qml
        LoadComponent.qml
        LocationComponent.qml
        MimeComponent.qml
        PdfPreviewComponent.qml
        PollComponent.qml
        QuoteComponent.qml
        ReactionComponent.qml
        ReplyAuthorComponent.qml
        ReplyButtonComponent.qml
        ReplyComponent.qml
        StateComponent.qml
        TextComponent.qml
        ThreadBodyComponent.qml
        VideoComponent.qml
    SOURCES
        contentprovider.cpp
        mediasizehelper.cpp
        pollhandler.cpp
        models/chatbarmessagecontentmodel.cpp
        models/itinerarymodel.cpp
        models/linemodel.cpp
        models/messagecontentmodel.cpp
        models/pollanswermodel.cpp
        models/reactionmodel.cpp
        models/threadmodel.cpp
        models/eventmessagecontentmodel.cpp
    RESOURCES
        images/bike.svg
        images/bus.svg
        images/cablecar.svg
        images/car.svg
        images/coach.svg
        images/couchettecar.svg
        images/elevator.svg
        images/escalator.svg
        images/ferry.svg
        images/flight.svg
        images/foodestablishment.svg
        images/funicular.svg
        images/longdistancetrain.svg
        images/rapidtransit.svg
        images/seat.svg
        images/shuttle.svg
        images/sleepingcar.svg
        images/stairs.svg
        images/subway.svg
        images/taxi.svg
        images/train.svg
        images/tramway.svg
        images/transfer.svg
        images/wait.svg
        images/walk.svg
    DEPENDENCIES
        QtQuick
        org.kde.neochat.libneochat
)

configure_file(config-neochat.h.in ${CMAKE_CURRENT_BINARY_DIR}/config-neochat.h)

ecm_qt_declare_logging_category(MessageContent
    HEADER "messagemodel_logging.h"
    IDENTIFIER "Message"
    CATEGORY_NAME "org.kde.neochat.messagemodel"
    DESCRIPTION "Neochat: messagemodel"
    DEFAULT_SEVERITY Info
    EXPORT NEOCHAT
)

target_include_directories(MessageContent PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(MessageContent PRIVATE
    Qt::Core
    Qt::Quick
    Qt::QuickControls2
    KF6::Kirigami
    LibNeoChat
)

if(NOT ANDROID)
    target_link_libraries(MessageContent PUBLIC KF6::SyntaxHighlighting)
endif()
