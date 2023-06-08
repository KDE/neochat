// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami

import org.kde.neochat 1.0
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

Kirigami.OverlaySheet {
    id: root

    title: i18n("Create a Space")

    Kirigami.Theme.colorSet: Kirigami.Theme.Window

    MobileForm.FormCard {
        contentItem: ColumnLayout {
            spacing: 0
            MobileForm.FormCardHeader {
                title: i18nc("@title", "Create a Space")
            }
            MobileForm.FormTextFieldDelegate {
                id: nameDelegate
                label: i18n("Space name")
            }
            MobileForm.FormTextFieldDelegate {
                id: topicDelegate
                label: i18n("Space topic (optional)")
            }
            MobileForm.FormButtonDelegate {
                text: i18n("Create space")
                onClicked: {
                    Controller.createSpace(nameDelegate.text, topicDelegate.text)
                    root.close()
                    root.destroy()
                }
                enabled: nameDelegate.text.length > 0
            }
        }
    }
}
