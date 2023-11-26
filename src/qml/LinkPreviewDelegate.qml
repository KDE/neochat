// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

Loader {
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
    property var defaultHeight : Kirigami.Units.gridUnit * 3 + Kirigami.Units.smallSpacing * 2

    /**
     * @brief Whether the loading indicator should animate if visible.
     */
    property bool indicatorEnabled: false

    visible: active
    sourceComponent: linkPreviewer && linkPreviewer.loaded ? linkPreviewComponent : loadingComponent

    Component {
        id: linkPreviewComponent
        QQC2.Control {
            id: componentRoot
            property bool truncated: linkPreviewDescription.truncated || !linkPreviewDescription.visible

            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Rectangle {
                    Layout.fillHeight: true
                    width: Kirigami.Units.smallSpacing
                    color: Kirigami.Theme.highlightColor
                }
                Image {
                    visible: root.linkPreviewer.imageSource
                    Layout.maximumHeight: root.defaultHeight
                    Layout.maximumWidth: root.defaultHeight
                    source: root.linkPreviewer.imageSource
                    fillMode: Image.PreserveAspectFit
                }
                ColumnLayout {
                    id: column
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
                            elideWidth: (linkPreviewTitle.width - Kirigami.Units.largeSpacing * 2.5) * 3
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
                visible: componentRoot.hovered && (componentRoot.truncated || checked)
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
    }

    Component {
        id: loadingComponent
        RowLayout {
            id: componentRoot
            property bool truncated: false

            Rectangle {
                Layout.fillHeight: true
                width: Kirigami.Units.smallSpacing
                color: Kirigami.Theme.highlightColor
            }
            QQC2.BusyIndicator {
                running: root.indicatorEnabled
            }
            Kirigami.Heading {
                Layout.fillWidth: true
                Layout.minimumHeight: root.defaultHeight
                level: 2
                text: i18n("Loading URL preview")
            }
        }
    }
}

