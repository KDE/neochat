// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show a link preview loading from a message.
 */
QQC2.Control {
    id: root

    /**
     * @brief The index of the delegate in the model.
     */
    required property int index

    /**
     * @brief Standard height for the link preview.
     *
     * When the content of the link preview is larger than this it will be
     * elided/hidden until maximized.
     */
    property var defaultHeight: Kirigami.Units.gridUnit * 3 + Kirigami.Units.smallSpacing * 2

    /**
     * @brief Request for this delegate to be removed.
     */
    signal remove(int index)

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    contentItem : RowLayout {
        spacing: Kirigami.Units.smallSpacing

        Rectangle {
            Layout.fillHeight: true
            width: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.highlightColor
        }
        QQC2.BusyIndicator {}
        Kirigami.Heading {
            Layout.fillWidth: true
            Layout.minimumHeight: root.defaultHeight
            verticalAlignment: Text.AlignVCenter
            level: 2
            text: i18nc("As in the app is loading the link preview", "Loading URL preview…")
        }
    }

    QQC2.Button {
        id: closeButton
        anchors.right: parent.right
        anchors.top: parent.top
        text: i18nc("As in remove the link preview so it's no longer shown", "Remove preview")
        icon.name: "dialog-close"
        display: QQC2.AbstractButton.IconOnly

        onClicked: {
            let sourceIndex = root.Message.contentFilterModel ?
                              root.Message.contentFilterModel.mapToSource(root.Message.contentFilterModel.index(root.index, 0)).row :
                              root.index
            root.remove(sourceIndex)
        }

        QQC2.ToolTip {
            text: closeButton.text
            visible: closeButton.hovered
            delay: Kirigami.Units.toolTipDelay
        }
    }
}
