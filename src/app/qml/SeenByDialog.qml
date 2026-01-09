// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

Kirigami.Dialog {
    id: root

    property var model

    standardButtons: Kirigami.Dialog.NoButton

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    maximumHeight: Kirigami.Units.gridUnit * 24
    title: i18nc("@title:menu Seen by/read marker dialog", "Seen By")

    contentItem: ColumnLayout {
        spacing: 0

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: listView

                model: root.model
                spacing: Kirigami.Units.smallSpacing

                onCountChanged: {
                    if (listView.count === 0) {
                        root.close();
                    }
                }

                delegate: Delegates.RoundedItemDelegate {
                    id: userDelegate

                    required property string displayName
                    required property url avatarUrl
                    required property color memberColor
                    required property string userId

                    implicitHeight: Kirigami.Units.gridUnit * 2

                    text: displayName
                    highlighted: false

                    onClicked: {
                        root.close();
                        RoomManager.resolveResource(userDelegate.userId);
                    }

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        KirigamiComponents.Avatar {
                            implicitWidth: height
                            sourceSize {
                                height: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                                width: Kirigami.Units.gridUnit + Kirigami.Units.smallSpacing * 2.5
                            }
                            source: userDelegate.avatarUrl
                            name: userDelegate.displayName
                            color: userDelegate.memberColor

                            Layout.fillHeight: true
                        }
                        QQC2.Label {
                            text: userDelegate.displayName
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            clip: true // Intentional to limit insane Unicode in display names

                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
