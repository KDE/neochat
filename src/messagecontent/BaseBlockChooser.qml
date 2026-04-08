// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.libneochat

/**
 * @brief Select a message component based on a Blocks.Type.
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
        roleValue: Blocks.Author
        delegate: AuthorComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Text
        delegate: TextComponent {
            onSelectedTextChanged: root.selectedTextChanged(selectedText)
            onHoveredLinkChanged: root.hoveredLinkChanged(hoveredLink)
        }
    }

    DelegateChoice {
        roleValue: Blocks.Image
        delegate: ImageComponent {
            rightAnchorMargin: root.rightAnchorMargin
        }
    }

    DelegateChoice {
        roleValue: Blocks.Video
        delegate: VideoComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Code
        delegate: CodeComponent {
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
        }
    }

    DelegateChoice {
        roleValue: Blocks.Quote
        delegate: QuoteComponent {
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
        }
    }

    DelegateChoice {
        roleValue: Blocks.Audio
        delegate: AudioComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.File
        delegate: FileComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Itinerary
        delegate: ItineraryComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Pdf
        delegate: PdfPreviewComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Poll
        delegate: PollComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Location
        delegate: LocationComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.LiveLocation
        delegate: LiveLocationComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Encrypted
        delegate: EncryptedComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Reply
        delegate: ReplyComponent {
            rightAnchorMargin: root.rightAnchorMargin
        }
    }

    DelegateChoice {
        roleValue: Blocks.Reaction
        delegate: ReactionComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.LinkPreview
        delegate: LinkPreviewComponent {
            onRemove: index => root.removeLinkPreview(index)
        }
    }

    DelegateChoice {
        roleValue: Blocks.LinkPreviewLoad
        delegate: LinkPreviewLoadComponent {
            type: LinkPreviewLoadComponent.LinkPreview
            onRemove: index => root.removeLinkPreview(index)
        }
    }

    DelegateChoice {
        roleValue: Blocks.ReplyButton
        delegate: ReplyButtonComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.FetchButton
        delegate: FetchButtonComponent {
            onFetchMoreEvents: root.fetchMoreEvents()
        }
    }

    DelegateChoice {
        roleValue: Blocks.Verification
        delegate: MimeComponent {
            mimeIconSource: "security-high"
            required property var model
            label: i18n("%1 started a user verification", model.author.htmlSafeDisplayName)
        }
    }

    DelegateChoice {
        roleValue: Blocks.Loading
        delegate: LoadComponent {}
    }

    DelegateChoice {
        roleValue: Blocks.Separator
        delegate: Kirigami.Separator {
            Layout.fillWidth: true
            Layout.maximumWidth: Message.maxContentWidth
        }
    }

    DelegateChoice {
        roleValue: Blocks.Other
        delegate: Item {}
    }
}
