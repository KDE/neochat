// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami
import org.kde.sonnet 1.0 as Sonnet
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

Kirigami.ScrollablePage {
    id: root

    FormCard.FormHeader {
        title: i18nc("@title", "Spellchecking")
    }
    FormCard.FormCard {
        id: card
        property Sonnet.Settings settings: Sonnet.Settings {
            id: settings
        }
        FormCard.FormCheckDelegate {
            id: enable
            checked: settings.checkerEnabledByDefault
            text: i18n("Enable automatic spell checking")
            onCheckedChanged: {
                settings.checkerEnabledByDefault = checked;
                settings.save();
            }
        }

        FormCard.FormDelegateSeparator { below: enable; above: skipUppercase }

        FormCard.FormCheckDelegate {
            id: skipUppercase
            checked: settings.skipUppercase
            text: i18n("Ignore uppercase words")
            onCheckedChanged: {
                settings.skipUppercase = checked;
                settings.save();
            }
        }

        FormCard.FormDelegateSeparator { below: skipUppercase; above: skipRunTogether }

        FormCard.FormCheckDelegate {
            id: skipRunTogether
            checked: settings.skipRunTogether
            text: i18n("Ignore hyphenated words")
            onCheckedChanged: {
                settings.skipRunTogether = checked;
                settings.save();
            }
        }

        FormCard.FormDelegateSeparator { below: skipRunTogether; above: autodetectLanguageCheckbox }

        FormCard.FormCheckDelegate {
            id: autodetectLanguageCheckbox
            checked: settings.autodetectLanguage
            text: i18n("Detect language automatically")
            onCheckedChanged: {
                settings.autodetectLanguage = checked;
                settings.save();
            }
        }

        FormCard.FormDelegateSeparator { below: autodetectLanguageCheckbox; above: selectedDefaultLanguage }

        FormCard.FormComboBoxDelegate {
            id: selectedDefaultLanguage
            text: i18n("Selected default language:")
            model: isEmpty ? [{"display": i18n("None")}] : settings.dictionaryModel
            textRole: "display"
            displayMode: Kirigami.Settings.isMobile ? FormCard.FormComboBoxDelegate.Dialog : FormCard.FormComboBoxDelegate.Page
            valueRole: "languageCode"
            property bool isEmpty: false
            Component.onCompleted: {
                if (settings.dictionaryModel.rowCount() === 0) {
                    isEmpty = true;
                } else {
                    currentIndex = indexOfValue(settings.defaultLanguage);
                }
            }
            onActivated: settings.defaultLanguage = currentValue;
        }

        FormCard.FormDelegateSeparator { below: selectedDefaultLanguage; above: spellCheckingLanguage }

        FormCard.FormButtonDelegate {
            id: spellCheckingLanguage
            text: i18n("Additional spell checking languages")
            description: i18n("%1 will provide spell checking and suggestions for the languages listed here when autodetection is enabled.", Qt.application.displayName)
            enabled: autodetectLanguageCheckbox.checked
            onClicked: pageStack.pushDialogLayer(spellCheckingLanguageList, {}, {
                width: pageStack.width - Kirigami.Units.gridUnit * 5,
                height: pageStack.height - Kirigami.Units.gridUnit * 5,
            })
        }

        FormCard.FormDelegateSeparator { below: spellCheckingLanguageList; above: personalDictionary }

        FormCard.FormButtonDelegate {
            id: personalDictionary
            text: i18n("Open Personal Dictionary")
            onClicked: pageStack.pushDialogLayer(dictionaryPage, {}, {
                width: pageStack.width - Kirigami.Units.gridUnit * 5,
                height: pageStack.height - Kirigami.Units.gridUnit * 5,
            })
        }

        property Component spellCheckingLanguageList: Component {
            id: spellCheckingLanguageList
            Kirigami.ScrollablePage {
                id: scroll
                title: i18nc("@title:window", "Spell checking languages")
                ListView {
                    clip: true
                    model: settings.dictionaryModel
                    delegate: Kirigami.CheckableListItem {
                        label: model.display
                        action: Kirigami.Action {
                            onTriggered: model.checked = checked
                        }
                        Accessible.description: model.isDefault ? i18n("Default Language") : ''
                        checked: model.checked
                        trailing: Kirigami.Icon {
                            source: "favorite"
                            visible: model.isDefault
                            HoverHandler {
                                id: hover
                            }
                            QQC2.ToolTip {
                                visible: hover.hovered
                                text: i18n("Default Language")
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: dictionaryPage
        Kirigami.ScrollablePage {
            title: i18n("Spell checking dictionary")
            footer: QQC2.ToolBar {
                contentItem: RowLayout {
                    QQC2.TextField {
                        id: dictionaryField
                        Layout.fillWidth: true
                        Accessible.name: placeholderText
                        placeholderText: i18n("Add a new word to your personal dictionary…")
                    }
                    QQC2.Button {
                        text: i18nc("@action:button", "Add word")
                        icon.name: "list-add"
                        enabled: dictionaryField.text.length > 0
                        onClicked: {
                            add(dictionaryField.text);
                            dictionaryField.clear();
                            if (instantApply) {
                                settings.save();
                            }
                        }
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                    }
                }
            }
            ListView {
                model: settings.currentIgnoreList
                delegate: Kirigami.BasicListItem {
                    label: model.modelData
                    trailing: QQC2.ToolButton {
                        icon.name: "delete"
                        onClicked: {
                            remove(modelData)
                            if (instantApply) {
                                settings.save();
                            }
                        }
                        QQC2.ToolTip {
                            text: i18n("Delete word")
                        }
                    }
                }
            }
        }
    }

    function add(word: string) {
        const dictionary = settings.currentIgnoreList;
        dictionary.push(word);
        settings.currentIgnoreList = dictionary;
    }

    function remove(word: string) {
        settings.currentIgnoreList = settings.currentIgnoreList.filter((value, _, _) => {
            return value !== word;
        });
    }
}
