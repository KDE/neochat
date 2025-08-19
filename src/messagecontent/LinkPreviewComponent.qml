// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-FileCopyrightText: 2023-2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show a link preview from a message.
 */
QQC2.Control {
    id: root

    /**
     * @brief The index of the delegate in the model.
     */
    required property int index

    /**
     * @brief The link preview properties.
     *
     * This is a list or object containing the following:
     *  - url - The URL being previewed.
     *  - loaded - Whether the URL preview has been loaded.
     *  - title - the title of the URL preview.
     *  - description - the description of the URL preview.
     *  - imageSource - a source URL for the preview image.
     */
    required property LinkPreviewer linkPreviewer

    /**
     * @brief Standard height for the link preview.
     *
     * When the content of the link preview is larger than this it will be
     * elided/hidden until maximized.
     */
    property var defaultHeight: Kirigami.Units.gridUnit * 3 + Kirigami.Units.largeSpacing * 2

    property bool truncated: linkPreviewDescription.truncated || !linkPreviewDescription.visible

    /**
     * @brief Request for this delegate to be removed.
     */
    signal remove(int index)

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth
    Layout.minimumHeight: root.defaultHeight

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    contentItem: RowLayout {
        id: contentRow
        spacing: Kirigami.Units.smallSpacing

        Rectangle {
            id: separator
            Layout.fillHeight: true
            width: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.highlightColor
            radius: Kirigami.Units.cornerRadius
        }
        Image {
            id: previewImage
            Layout.preferredWidth: root.defaultHeight
            Layout.preferredHeight: root.defaultHeight
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.linkPreviewer.imageSource.toString().length > 0
            source: root.linkPreviewer.imageSource
            fillMode: Image.PreserveAspectFit
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
        }
        ColumnLayout {
            id: column
            implicitWidth: Math.max(linkPreviewTitle.implicitWidth, linkPreviewDescription.implicitWidth)
            spacing: Kirigami.Units.smallSpacing
            visible: root.linkPreviewer.title.length > 0 || root.linkPreviewer.description.length > 0
            Kirigami.Heading {
                id: linkPreviewTitle
                Layout.fillWidth: true
                level: 3
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                text: "<style>
                    a {
                        text-decoration: none;
                    }
                    </style>
                    <a href=\"" + root.linkPreviewer.url + "\">" + (maximizeButton.checked ? root.linkPreviewer.title : titleTextMetrics.elidedText).replace("&ndash;", "—") + "</a>"

                TextMetrics {
                    id: titleTextMetrics
                    text: root.linkPreviewer.title
                    font: linkPreviewTitle.font
                    elide: Text.ElideRight
                    elideWidth: linkPreviewTitle.width
                }
            }
            QQC2.Label {
                id: linkPreviewDescription
                Layout.fillWidth: true
                Layout.maximumHeight: maximizeButton.checked ? -1 : root.defaultHeight - linkPreviewTitle.height - column.spacing
                visible: linkPreviewTitle.height + column.spacing + font.pointSize <= root.defaultHeight || maximizeButton.checked
                text: linkPreviewer.description
                wrapMode: Text.Wrap
                elide: Text.ElideRight
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: RoomManager.resolveResource(root.linkPreviewer.url, "join")
        }
    }

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    QQC2.Button {
        id: closeButton
        anchors.right: parent.right
        anchors.top: parent.top
        visible: root.hovered
        text: i18nc("As in remove the link preview so it's no longer shown", "Remove preview")
        icon.name: "dialog-close"
        display: QQC2.AbstractButton.IconOnly

        onClicked: root.remove(root.index)

        QQC2.ToolTip {
            text: closeButton.text
            visible: closeButton.hovered
            delay: Kirigami.Units.toolTipDelay
        }
    }
    QQC2.Button {
        id: maximizeButton
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: root.hovered && (root.truncated || checked)
        checkable: true
        text: checked ? i18n("Shrink preview") : i18n("Expand preview")
        icon.name: checked ? "go-up" : "go-down"
        display: QQC2.AbstractButton.IconOnly

        QQC2.ToolTip {
            text: maximizeButton.text
            visible: maximizeButton.hovered
            delay: Kirigami.Units.toolTipDelay
        }
    }
}
