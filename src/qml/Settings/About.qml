// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18nc("@title:window", "About NeoChat")
    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        MobileForm.AboutCard {
            Layout.fillWidth: true
            aboutData: Controller.aboutData
        }
    }
}
