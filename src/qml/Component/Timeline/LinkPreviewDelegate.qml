// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Loader {
    id: root

    /**
     * @brief Get a list of hyperlinks in the text.
     *
     * User links i.e. anything starting with https://matrix.to are ignored.
     */
    property var links: {
        let matches = model.display.match(/\bhttps?:\/\/[^\s\<\>\"\']+/g)
        if (matches && matches.length > 0) {
            // don't show previews for room links or user mentions or custom emojis
            return matches.filter(link => !(
                link.includes("https://matrix.to") || link.includes("/_matrix/media/r0/download/")
            ))
            // remove ending fullstops and commas
            .map(link => (link.length && [".", ","].includes(link[link.length-1])) ? link.substring(0, link.length-1) : link)
        }
        return []

    }
    LinkPreviewer {
        id: linkPreviewer
        url: root.links && root.links.length > 0 ? root.links[0] : ""
    }

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

    active: !currentRoom.usesEncryption && model.display && links && links.length > 0 && currentRoom.urlPreviewEnabled
    visible: Config.showLinkPreview && active
    sourceComponent: linkPreviewer.loaded ? linkPreviewComponent : loadingComponent

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
                    visible: linkPreviewer.imageSource
                    Layout.maximumHeight: root.defaultHeight
                    Layout.maximumWidth: root.defaultHeight
                    source: linkPreviewer.imageSource.replace("mxc://", "image://mxc/")
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
                        <a href=\"" + root.links[0] + "\">" + (maximizeButton.checked ? linkPreviewer.title : titleTextMetrics.elidedText).replace("&ndash;", "—") + "</a>"
                        onLinkActivated: RoomManager.openResource(link)

                        TextMetrics {
                            id: titleTextMetrics
                            text: linkPreviewer.title
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
                icon.name: checked ? "go-up" : "go-down"

                QQC2.ToolTip.text: checked ? i18n("Shrink preview") : i18n("Expand preview")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
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

