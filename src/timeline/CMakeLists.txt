# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(timeline STATIC)
qt_add_qml_module(timeline
    URI org.kde.neochat.timeline
    QML_FILES
        EventDelegate.qml
        TimelineDelegate.qml
        MessageDelegate.qml
        LoadingDelegate.qml
        ReadMarkerDelegate.qml
        StateDelegate.qml
        TimelineEndDelegate.qml
        Bubble.qml
        AvatarFlow.qml
        ReactionDelegate.qml
        SectionDelegate.qml
        MessageComponentChooser.qml
        AudioComponent.qml
        CodeComponent.qml
        EncryptedComponent.qml
        FileComponent.qml
        ImageComponent.qml
        ItineraryComponent.qml
        LinkPreviewComponent.qml
        LiveLocationComponent.qml
        LoadComponent.qml
        LocationComponent.qml
        MessageEditComponent.qml
        MimeComponent.qml
        PdfPreviewComponent.qml
        PollComponent.qml
        QuoteComponent.qml
        ReplyComponent.qml
        StateComponent.qml
        TextComponent.qml
        VideoComponent.qml
)
