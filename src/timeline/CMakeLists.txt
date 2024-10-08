# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(timeline STATIC)
ecm_add_qml_module(timeline GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.timeline
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/timeline
    QML_FILES
        EventDelegate.qml
        HiddenDelegate.qml
        MessageDelegate.qml
        LoadingDelegate.qml
        PredecessorDelegate.qml
        ReadMarkerDelegate.qml
        StateDelegate.qml
        SuccessorDelegate.qml
        TimelineEndDelegate.qml
        Bubble.qml
        AvatarFlow.qml
        ReactionDelegate.qml
        SectionDelegate.qml
        MessageComponentChooser.qml
        ReplyMessageComponentChooser.qml
        AuthorComponent.qml
        AudioComponent.qml
        ChatBarComponent.qml
        CodeComponent.qml
        EncryptedComponent.qml
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
        ReplyAuthorComponent.qml
        ReplyComponent.qml
        StateComponent.qml
        TextComponent.qml
        VideoComponent.qml
    SOURCES
        timelinedelegate.cpp
        timelinedelegate.h
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
)

target_link_libraries(timeline PRIVATE
    Qt::Quick
    KF6::Kirigami
)
