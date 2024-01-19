// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

/**
 * @brief Component for a generic search page.
 *
 * This component provides a header with the search field and a ListView to visualise
 * search results from the given model.
 */
Kirigami.ScrollablePage {
    id: root

    /**
     * @brief Any additional controls after the search button.
     */
    property alias headerTrailing: headerContent.children

    /**
     * @brief The model that provides the search results.
     *
     * The model needs to provide the following properties:
     *      - searchText
     *      - searching
     * Where searchText is the text from the searchField and is used to match results
     * and searching is true while the model is finding results.
     *
     * The model must also provide a search() function to start the search if
     * it doesn't do so when the searchText is changed.
     */
    property alias model: listView.model

    /**
     * @brief The number of delegates currently in the view.
     */
    property alias count: listView.count

    /**
     * @brief The delegate to use to visualize the model data.
     */
    property alias modelDelegate: listView.delegate

    /**
     * @brief The delegate to appear as the header of the list.
     */
    property alias listHeaderDelegate: listView.header

    /**
     * @brief The delegate to appear as the footer of the list.
     */
    property alias listFooterDelegate: listView.footer

    /**
     * @brief The placeholder text in the search field.
     */
    property alias searchFieldPlaceholder: searchField.placeholderText

    /**
     * @brief The text to show when no search term has been entered.
     */
    property alias noSearchPlaceholderMessage: noSearchMessage.text

    /**
     * @brief The text to show when no results have been found.
     */
    property alias noResultPlaceholderMessage: noResultMessage.text

    /**
     * @brief The verticalLayoutDirection property of the internal ListView.
     */
    property alias listVerticalLayoutDirection: listView.verticalLayoutDirection

    /**
     * @brief Force the search field to be focussed.
     */
    function focusSearch() {
        searchField.forceActiveFocus();
    }

    header: QQC2.Control {
        padding: Kirigami.Units.largeSpacing

        background: Rectangle {
            Kirigami.Theme.colorSet: Kirigami.Theme.Window
            Kirigami.Theme.inherit: false
            color: Kirigami.Theme.backgroundColor

            Kirigami.Separator {
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                    right: parent.right
                }
            }
        }

        contentItem: RowLayout {
            id: headerContent
            spacing: Kirigami.Units.largeSpacing

            Kirigami.SearchField {
                id: searchField
                focus: true
                Layout.fillWidth: true
                Keys.onEnterPressed: searchButton.clicked()
                Keys.onReturnPressed: searchButton.clicked()
                onTextChanged: {
                    if (model) {
                        model.searchText = text;
                    }
                }
            }
            QQC2.Button {
                id: searchButton
                icon.name: "search"
                onClicked: {
                    if (typeof model.search === 'function') {
                        model.search()
                    }
                }
            }
        }
    }

    ListView {
        id: listView
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 0

        section.property: "section"

        Kirigami.PlaceholderMessage {
            id: noSearchMessage
            anchors.centerIn: parent
            visible: searchField.text.length === 0 && listView.count === 0
        }

        Kirigami.PlaceholderMessage {
            id: noResultMessage
            anchors.centerIn: parent
            visible: searchField.text.length > 0 && listView.count === 0 && !root.model.searching
        }

        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            visible: root.model.searching
        }
    }
}

