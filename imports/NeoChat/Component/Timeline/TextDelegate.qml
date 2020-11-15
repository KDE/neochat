/**
 * SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2

import org.kde.kirigami 2.4 as Kirigami

QQC2.Label {
    text: "<style>pre {white-space: pre-wrap} a{color: " + Kirigami.Theme.linkColor + ";} .user-pill{}</style>" + display

    font.family: Kirigami.Theme.defaultFont.family + ", emoji"

    wrapMode: Text.WordWrap
    width: parent.width
    textFormat: Text.RichText
}

