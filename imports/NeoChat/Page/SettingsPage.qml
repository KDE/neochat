/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as QQC2
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Settings")

    Kirigami.FormLayout {
        QQC2.CheckBox {
            id: showNotifications
            Kirigami.FormData.label: i18n("Show Notifications:")
            checked: Config.showNotifications
        }
    }

    footer: RowLayout {
        QQC2.Button {
            Layout.alignment: Qt.AlignRight
            Layout.margins: Kirigami.Units.smallSpacing
            text: i18n("Save")
            onClicked: {
                Config.showNotifications = showNotifications.checked;
                Config.save();
            }
        }
    }
}
