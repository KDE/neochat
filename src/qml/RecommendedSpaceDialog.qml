// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.components

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property var connection

    title: i18nc("@title Join <name of a space>", "Join %1", SpaceHierarchyCache.recommendedSpaceDisplayName)

    FormCard.FormHeader {
        title: i18nc("@title", "Your distro recommends you join this space")
    }
    FormCard.FormCard {
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
                SpaceHierarchyCache.recommendedSpaceHidden = true
                RoomManager.resolveResource(SpaceHierarchyCache.recommendedSpaceId, "join")
                root.closeDialog()
            }
        }
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Ignore")
            onClicked: {
                SpaceHierarchyCache.recommendedSpaceHidden = true
                root.closeDialog()
            }
        }
    }
}
