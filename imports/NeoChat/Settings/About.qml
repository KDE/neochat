// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.AboutPage {
    title: i18nc('@title:window', 'About NeoChat')
    aboutData: Controller.aboutData
}
