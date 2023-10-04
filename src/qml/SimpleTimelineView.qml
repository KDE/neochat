// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component for viewing a message timeline with only basic features.
 *
 * This component displays a list of messages, but is designed for features like
 * search and threads which don't require the full feature set of TimelineView.
 *
 * The component expects a model which needs to support the same set of roles as
 * MessageEventModel in order for all delegate types to work. The model must also
 * have a loading Q_PROPERTY.
 *
 * @sa TimelineView, MessageEventModel
 */
ListView {
    id: root

    /**
     * @brief The current active connection.
     */
    required property NeoChatConnection connection

    /**
     * @brief A placeholder message to be shown.
     *
     * This will only be visible if the model is empty and not loading.
     */
    property alias placeHolderMessage: placeholderMessage.text

    spacing: 0
    bottomMargin: Kirigami.Units.largeSpacing + Math.round(Kirigami.Theme.defaultFont.pointSize * 2)
    verticalLayoutDirection: ListView.BottomToTop
    clip: true

    // Not rendered because the sections are part of the MessageDelegate.qml, this is only so that items have the section property available for use by sectionBanner.
    // This is due to the fact that the ListView verticalLayout is BottomToTop.
    // This also flips the sections which would appear at the bottom but for a timeline they still need to be at the top (bottom from the qml perspective).
    // There is currently no option to put section headings at the bottom in qml.
    section.property: "section"

    Kirigami.PlaceholderMessage {
        id: placeholderMessage
        anchors.centerIn: parent
        visible: !root.model.loading && root.count === 0
    }

    Kirigami.LoadingPlaceholder {
        anchors.centerIn: parent
        visible: model.loading
    }

    delegate: EventDelegate {
        connection: root.connection
    }
}
