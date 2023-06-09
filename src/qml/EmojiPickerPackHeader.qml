// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.ScrollView {
    id: root

    QQC2.ScrollBar.horizontal.height: QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.implicitHeight : 0

    required property var model
    property int currentIndex: 0
    readonly property string currentCategory: root.model.data(root.model.index(root.currentIndex, 0), ImageContentPackRole.IdentifierRole)

    implicitHeight: Kirigami.Units.iconSizes.large + root.QQC2.ScrollBar.horizontal.height

    ListView {
        id: categories
        clip: true
        focus: true
        orientation: ListView.Horizontal
        currentIndex: root.currentIndex

        Keys.onReturnPressed: if (emojiGrid.count > 0) emojiGrid.focus = true
        Keys.onEnterPressed: if (emojiGrid.count > 0) emojiGrid.focus = true

        KeyNavigation.down: emojiGrid.count > 0 ? emojiGrid : categories
        KeyNavigation.tab: emojiGrid.count > 0 ? emojiGrid : categories

        keyNavigationEnabled: true
        keyNavigationWraps: true
        Keys.forwardTo: searchField
        interactive: width !== contentWidth

        Component.onCompleted: categories.forceActiveFocus()

        model: root.model

        delegate: Kirigami.NavigationTabButton {
            width: Kirigami.Units.iconSizes.large
            height: width
            checked: categories.currentIndex === model.index
            text: model.icon
            QQC2.ToolTip.text: model.displayName
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            QQC2.ToolTip.visible: hovered
            onClicked: {
                root.currentIndex = index;
            }
        }
    }
}
