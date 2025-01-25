// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief Select a message component based on a MessageComponentType.
 */
DelegateChooser {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The index of the delegate in the model.
     */
    required property var index

    /**
     * @brief The timeline ListView this component is being used in.
     */
    required property ListView timeline

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked(string eventID)

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    /**
     * @brief The user hovered link has changed.
     */
    signal hoveredLinkChanged(string hoveredLink)

    /**
     * @brief Request a context menu be show for the message.
     */
    signal showMessageMenu

    signal removeLinkPreview(int index)

    /**
     * @brief Request more events in the thread be loaded.
     */
    signal fetchMoreEvents()

    role: "componentType"

    DelegateChoice {
        roleValue: MessageComponentType.Author
        delegate: AuthorComponent {
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Text
        delegate: TextComponent {
            maxContentWidth: root.maxContentWidth
            onSelectedTextChanged: root.selectedTextChanged(selectedText)
            onHoveredLinkChanged: root.hoveredLinkChanged(hoveredLink)
            onShowMessageMenu: root.showMessageMenu()
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Image
        delegate: ImageComponent {
            room: root.room
            index: root.index
            timeline: root.timeline
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Video
        delegate: VideoComponent {
            room: root.room
            index: root.index
            timeline: root.timeline
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Code
        delegate: CodeComponent {
            maxContentWidth: root.maxContentWidth
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
            onShowMessageMenu: root.showMessageMenu()
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Quote
        delegate: QuoteComponent {
            maxContentWidth: root.maxContentWidth
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
            onShowMessageMenu: root.showMessageMenu()
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Audio
        delegate: AudioComponent {
            room: root.room
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.File
        delegate: FileComponent {
            room: root.room
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Itinerary
        delegate: ItineraryComponent {
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Pdf
        delegate: PdfPreviewComponent {
            maxContentWidth: root.maxContentWidth
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
        roleValue: MessageComponentType.Reply
        delegate: ReplyComponent {
            maxContentWidth: root.maxContentWidth
            onReplyClicked: eventId => {
                root.replyClicked(eventId);
            }
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.LinkPreview
        delegate: LinkPreviewComponent {
            maxContentWidth: root.maxContentWidth
            onRemove: index => root.removeLinkPreview(index)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.LinkPreviewLoad
        delegate: LinkPreviewLoadComponent {
            type: LinkPreviewLoadComponent.LinkPreview
            maxContentWidth: root.maxContentWidth
            onRemove: index => root.removeLinkPreview(index)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.ChatBar
        delegate: ChatBarComponent {
            room: root.room
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.ReplyButton
        delegate: ReplyButtonComponent {
            room: root.room
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.FetchButton
        delegate: FetchButtonComponent {
            maxContentWidth: root.maxContentWidth
            onFetchMoreEvents: root.fetchMoreEvents()
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Verification
        delegate: MimeComponent {
            mimeIconSource: "security-high"
            label: i18n("%1 started a user verification", model.author.htmlSafeDisplayName)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Loading
        delegate: LoadComponent {
            maxContentWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Separator
        delegate: Kirigami.Separator {
            Layout.fillWidth: true
            Layout.maximumWidth: root.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Other
        delegate: Item {}
    }
}
