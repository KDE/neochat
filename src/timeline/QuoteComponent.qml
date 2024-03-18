// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

QQC2.Control {
    id: root

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    /**
     * @brief Request a context menu be show for the message.
     */
    signal showMessageMenu

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.maximumWidth: root.maxContentWidth

    topPadding: 0
    bottomPadding: 0

    contentItem: TextEdit {
        id: quoteText
        Layout.fillWidth: true
        topPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing

        text: root.display
        readOnly: true
        textFormat: TextEdit.RichText
        wrapMode: TextEdit.Wrap
        color: Kirigami.Theme.textColor

        font.italic: true

        onSelectedTextChanged: root.selectedTextChanged(selectedText)

        TapHandler {
            enabled: !quoteText.hoveredLink
            acceptedButtons: Qt.LeftButton
            onLongPressed: root.showMessageMenu()
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing
    }
}
