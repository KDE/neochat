// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components

import org.kde.neochat

Kirigami.Dialog {
    id: root

    property var connection

    parent: applicationWindow().overlay

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    title: i18nc("@title Join <name of a space>", "Join %1", SpaceHierarchyCache.recommendedSpaceDisplayName)

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)

    contentItem: ColumnLayout {
        FormCard.AbstractFormDelegate {
            background: null
            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing * 4
                Avatar {
                    source: root.connection.makeMediaUrl(SpaceHierarchyCache.recommendedSpaceAvatar)
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Kirigami.Heading {
                        Layout.fillWidth: true
                        text: SpaceHierarchyCache.recommendedSpaceDisplayName
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        text: SpaceHierarchyCache.recommendedSpaceDescription
                    }
                }
            }
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Join")
            onClicked: {
                SpaceHierarchyCache.recommendedSpaceHidden = true;
                RoomManager.resolveResource(SpaceHierarchyCache.recommendedSpaceId, "join");
                root.close();
            }
        }
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Ignore")
            onClicked: {
                SpaceHierarchyCache.recommendedSpaceHidden = true;
                root.close();
            }
        }
    }
}
