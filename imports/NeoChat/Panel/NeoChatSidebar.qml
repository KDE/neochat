/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.12 as Kirigami

import SortFilterProxyModel 0.2

import NeoChat.Component 1.0
import NeoChat.Component.Timeline 1.0
import org.kde.neochat 1.0

Kirigami.GlobalDrawer {
    id: root
    
    modal: true
    collapsible: true
    collapsed: Kirigami.Settings.isMobile
}
