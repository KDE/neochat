// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-FileCopyrightText: 2023-2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

/**
 * @brief A component to show a link preview from a message.
 */
QQC2.Control {
    id: root

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
    required property var linkPreviewer

    /**
     * @brief Standard height for the link preview.
     *
     * When the content of the link preview is larger than this it will be
     * elided/hidden until maximized.
     */
    property var defaultHeight: Kirigami.Units.gridUnit * 3 + Kirigami.Units.smallSpacing * 2

    property bool truncated: linkPreviewDescription.truncated || !linkPreviewDescription.visible

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

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
        }
        Image {
            id: previewImage
            Layout.preferredWidth: root.defaultHeight
            Layout.preferredHeight: root.defaultHeight
            visible: root.linkPreviewer.imageSource.length > 0
            source: root.linkPreviewer.imageSource
            fillMode: Image.PreserveAspectFit
        }
        ColumnLayout {
            id: column
            implicitWidth: Math.max(linkPreviewTitle.implicitWidth, linkPreviewDescription.implicitWidth)
            spacing: Kirigami.Units.smallSpacing
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
                onLinkActivated: RoomManager.resolveResource(link, "join")

                TextMetrics {
                    id: titleTextMetrics
                    text: root.linkPreviewer.title
                    font: linkPreviewTitle.font
                    elide: Text.ElideRight
                    elideWidth: (linkPreviewTitle.availableWidth()) * 3
                }

                function availableWidth() {
                    let previewImageWidth = (previewImage.visible ? previewImage.width + contentRow.spacing : 0);
                    return root.maxContentWidth - contentRow.spacing - separator.width - previewImageWidth;
                }
            }
            QQC2.Label {
                id: linkPreviewDescription
                Layout.fillWidth: true
                Layout.maximumHeight: maximizeButton.checked ? -1 : root.defaultHeight - linkPreviewTitle.height - column.spacing
                visible: linkPreviewTitle.height + column.spacing <= root.defaultHeight || maximizeButton.checked
                text: linkPreviewer.description
                wrapMode: Text.Wrap
                elide: Text.ElideRight
            }
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
            visible: hovered
            delay: Kirigami.Units.toolTipDelay
        }
    }
}
