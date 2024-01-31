// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

/// Step for the login/registration flow
ColumnLayout {
    id: root

    /// Set to true if the login step does not have any controls. This will ensure that the focus remains on the "continue" button
    property bool noControls: false

    /// Process this module, this is called by the continue button.
    /// Should call \sa processed when it finish successfully.
    property QQC2.Action nextAction: null

    /// Go to the previous module. This is called by the "go back" button.
    /// If no "go back" button should be shown, this should be null.
    property QQC2.Action previousAction: null

    /// Called when switching to the next step.
    signal processed(url nextUrl)

    /// Show a message in a banner at the top of the page.
    signal showMessage(string message)

    /// Clears any error messages currently being shown
    signal clearError

    /// Closes the login dialog
    signal closeDialog
}
