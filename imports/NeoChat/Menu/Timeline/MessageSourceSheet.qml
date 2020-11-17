/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12

import org.kde.kirigami 2.12 as Kirigami

Kirigami.OverlaySheet {

    property string sourceText

    header: Kirigami.Heading {
        text: i18n("Message Source")
    }

    Label {
        text: sourceText
        wrapMode: Text.WordWrap
    }
}

