/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
pragma Singleton
import QtQuick 2.12
import Qt.labs.settings 1.0

Settings {
    property bool showNotification: true

    property bool showTray: true

    property bool darkTheme

    property string fontFamily: "Roboto,Noto Sans,Noto Color Emoji"
}
