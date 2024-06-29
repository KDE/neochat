// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.neochat

/**
 * @brief Select a message component based on a MessageComponentType.
 */
DelegateChooser {
    id: root

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked()

    role: "componentType"

    DelegateChoice {
        roleValue: MessageComponentType.Author
        delegate: ReplyAuthorComponent {
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Text
        delegate: TextComponent {
            maxContentWidth: root.maxContentWidth

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: root.replyClicked()
            }
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Image
        delegate: Image {
            id: image

            required property var mediaInfo

            Layout.maximumWidth: mediaSizeHelper.currentSize.width
            Layout.maximumHeight: mediaSizeHelper.currentSize.height
            source: image.mediaInfo.source

            MediaSizeHelper {
                id: mediaSizeHelper
                contentMaxWidth: root.maxContentWidth
                mediaWidth: image.mediaInfo.width ?? 0
                mediaHeight: image.mediaInfo.height ?? 0
            }
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Video
        delegate: MimeComponent {
            required property string display
            required property var mediaInfo
            required property int componentType

            mimeIconSource: mediaInfo.mimeIcon
            label: display
            subLabel: componentType === MessageComponentType.File ? Format.formatByteSize(mediaInfo.size) : Format.formatDuration(mediaInfo.duration)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Code
        delegate: CodeComponent {
            maxContentWidth: root.maxContentWidth

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: root.replyClicked()
            }
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Quote
        delegate: QuoteComponent {
            maxContentWidth: root.maxContentWidth

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: root.replyClicked()
            }
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Audio
        delegate: MimeComponent {
            required property string display
            required property var mediaInfo
            required property int componentType

            mimeIconSource: mediaInfo.mimeIcon
            label: display
            subLabel: componentType === MessageComponentType.File ? Format.formatByteSize(mediaInfo.size) : Format.formatDuration(mediaInfo.duration)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.File
        delegate: MimeComponent {
            required property string display
            required property var mediaInfo
            required property int componentType

            mimeIconSource: mediaInfo.mimeIcon
            label: display
            subLabel: componentType === MessageComponentType.File ? Format.formatByteSize(mediaInfo.size) : Format.formatDuration(mediaInfo.duration)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Poll
        delegate: PollComponent {
            room: root.room
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Location
        delegate: LocationComponent {
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.LiveLocation
        delegate: LiveLocationComponent {
            room: root.room
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Encrypted
        delegate: EncryptedComponent {
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Loading
        delegate: LoadComponent {
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Other
        delegate: Item {}
    }
}
