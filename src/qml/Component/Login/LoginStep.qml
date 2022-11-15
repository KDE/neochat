// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.14
import QtQuick.Controls 2.14 as QQC2
import QtQuick.Layouts 1.14

/// Step for the login/registration flow
ColumnLayout {

    property string title: i18n("Welcome")
    property string message: i18n("Welcome")
    property bool showContinueButton: false
    property bool showBackButton: false
    property bool acceptable: false
    property string previousUrl: ""

    /// Process this module, this is called by the continue button.
    /// Should call \sa processed when it finish successfully.
    property QQC2.Action action: null

    /// Called when switching to the next step.
    signal processed(url nextUrl)

    signal showMessage(string message)

}
