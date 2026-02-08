// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

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
     * @brief Extra margin required when anchoring an item on the right.
     *
     * Normally used for scrollbars.
     */
    property int rightAnchorMargin: 0

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    /**
     * @brief The user hovered link has changed.
     */
    signal hoveredLinkChanged(string hoveredLink)

    signal removeLinkPreview(int index)

    /**
     * @brief Request more events in the thread be loaded.
     */
    signal fetchMoreEvents()

    role: "componentType"

    DelegateChoice {
        roleValue: MessageComponentType.Author
        delegate: AuthorComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Text
        delegate: TextComponent {
            onSelectedTextChanged: root.selectedTextChanged(selectedText)
            onHoveredLinkChanged: root.hoveredLinkChanged(hoveredLink)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Image
        delegate: ImageComponent {
            rightAnchorMargin: root.rightAnchorMargin
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Video
        delegate: VideoComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Code
        delegate: CodeComponent {
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Quote
        delegate: QuoteComponent {
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Audio
        delegate: AudioComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.File
        delegate: FileComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Itinerary
        delegate: ItineraryComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Pdf
        delegate: PdfPreviewComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Poll
        delegate: PollComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Location
        delegate: LocationComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.LiveLocation
        delegate: LiveLocationComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Encrypted
        delegate: EncryptedComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Reply
        delegate: ReplyComponent {
            rightAnchorMargin: root.rightAnchorMargin
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Reaction
        delegate: ReactionComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.LinkPreview
        delegate: LinkPreviewComponent {
            onRemove: index => root.removeLinkPreview(index)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.LinkPreviewLoad
        delegate: LinkPreviewLoadComponent {
            type: LinkPreviewLoadComponent.LinkPreview
            onRemove: index => root.removeLinkPreview(index)
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.ReplyButton
        delegate: ReplyButtonComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.FetchButton
        delegate: FetchButtonComponent {
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
        delegate: LoadComponent {}
    }

    DelegateChoice {
        roleValue: MessageComponentType.Separator
        delegate: Kirigami.Separator {
            Layout.fillWidth: true
            Layout.maximumWidth: Message.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: MessageComponentType.Other
        delegate: Item {}
    }
}
