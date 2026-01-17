// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat.libneochat as LibNeoChat

QQC2.Popup {
    id: root

    property alias model: completions.model

    required property LibNeoChat.ChatKeyHelper keyHelper

    Connections {
        target: keyHelper

        function onUnhandledUp(isCompleting: bool): void {
            if (!isCompleting) {
                return;
            }
            root.decrementIndex();
        }

        function onUnhandledDown(isCompleting: bool): void {
            if (!isCompleting) {
                return;
            }
            root.incrementIndex();
        }

        function onUnhandledTab(isCompleting: bool): void {
            if (!isCompleting) {
                return;
            }
            root.completeCurrent();
        }

        function onUnhandledReturn(isCompleting: bool): void {
            if (!isCompleting) {
                return;
            }
            root.completeCurrent();
        }

        function onCloseCompletion(): void {
            root.close();
            root.model.ignoreCurrentCompletion();
        }
    }

    x: model.textItem.textItem.cursorRectangle.x - Kirigami.Units.largeSpacing
    y: model.textItem.textItem.cursorRectangle.y - implicitHeight - Kirigami.Units.smallSpacing

    function incrementIndex() {
        completions.incrementCurrentIndex();
    }

    function decrementIndex() {
        completions.decrementCurrentIndex();
    }

    function complete(text: string, hRef: string) {
        model.insertCompletion(text, hRef);
    }

    function completeCurrent() {
        model.insertCompletion(completions.currentItem.replacedText, completions.currentItem.hRef);
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    implicitHeight: Math.min(completions.contentHeight, Kirigami.Units.gridUnit * 10)

    contentItem: QQC2.ScrollView {
        contentWidth: Kirigami.Units.gridUnit * 20

        ListView {
            id: completions
            currentIndex: 0
            keyNavigationWraps: true
            highlightMoveDuration: 100
            onCountChanged: currentIndex = 0
            delegate: Delegates.RoundedItemDelegate {
                id: completionDelegate

                required property int index
                required property string displayName
                required property string subtitle
                required property string iconName
                required property string replacedText
                required property url hRef

                text: displayName

                contentItem: RowLayout {
                    KirigamiComponents.Avatar {
                        visible: completionDelegate.iconName !== "invalid"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                        source: completionDelegate.iconName === "invalid" ? "" : completionDelegate.iconName
                        name: completionDelegate.text
                    }
                    Delegates.SubtitleContentItem {
                        itemDelegate: completionDelegate
                        labelItem.textFormat: Text.PlainText
                        labelItem.clip: true // Intentional to limit insane Unicode in display names
                        subtitle: completionDelegate.subtitle ?? ""
                        subtitleItem.textFormat: Text.PlainText
                    }
                }
                onClicked: root.model.insertCompletion(replacedText, hRef)
            }
        }
    }

    background: Kirigami.ShadowedRectangle {
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        radius: Kirigami.Units.cornerRadius
        color: Kirigami.Theme.backgroundColor
        border {
            width: 1
            color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
        }
    }
}
