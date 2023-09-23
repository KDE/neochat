// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import org.kde.kirigami as Kirigami

Kirigami.Action {
    property var inputData: ({})
    property var doBeforeSharing: () => {}
    visible: false
}
