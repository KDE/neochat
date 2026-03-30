// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief Select a message component based on a Blocks.Type.
 */
DelegateChooser {
    id: root

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked()

    role: "componentType"

    DelegateChoice {
        roleValue: Blocks.Author
        delegate: ReplyAuthorComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Text
        delegate: TextComponent {
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: root.replyClicked()
            }
        }
    }

    DelegateChoice {
        roleValue: Blocks.Image
        delegate: ImageComponent {
            contentMaxHeight: Kirigami.Units.gridUnit * 5
        }
    }

    DelegateChoice {
        roleValue: Blocks.Video
        delegate: MimeComponent {
            required property var componentAttributes
            
            mimeIconSource: componentAttributes.mimeIcon
            size: componentAttributes.size
            duration: componentAttributes.duration
            label: componentAttributes.filename
        }
    }

    DelegateChoice {
        roleValue: Blocks.Code
        delegate: CodeComponent {
            Layout.maximumHeight: Kirigami.Units.gridUnit * 5

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: root.replyClicked()
            }
        }
    }

    DelegateChoice {
        roleValue: Blocks.Quote
        delegate: QuoteComponent {
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: root.replyClicked()
            }
        }
    }

    DelegateChoice {
        roleValue: Blocks.Audio
        delegate: MimeComponent {
            required property string display
            required property var componentAttributes

            mimeIconSource: componentAttributes.mimeIcon
            size: componentAttributes.size
            duration: componentAttributes.duration
            label: componentAttributes.filename
        }
    }

    DelegateChoice {
        roleValue: Blocks.File
        delegate: MimeComponent {
            required property string display
            required property var componentAttributes

            mimeIconSource: componentAttributes.mimeIcon
            size: componentAttributes.size
            label: componentAttributes.filename
        }
    }

    DelegateChoice {
        roleValue: Blocks.Poll
        delegate: PollComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Location
        delegate: MimeComponent {
            required property string display
            mimeIconSource: "mark-location"
            label: display
        }
    }

    DelegateChoice {
        roleValue: Blocks.LiveLocation
        delegate: MimeComponent {
            required property string display
            mimeIconSource: "mark-location"
            label: display
        }
    }

    DelegateChoice {
        roleValue: Blocks.Encrypted
        delegate: EncryptedComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Loading
        delegate: LoadComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Other
        delegate: Item {}
    }
}
