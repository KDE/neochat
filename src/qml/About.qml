// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick.Layouts
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.neochat

FormCard.AboutPage {
    title: i18nc("@title:window", "About NeoChat")
    aboutData: About
}
