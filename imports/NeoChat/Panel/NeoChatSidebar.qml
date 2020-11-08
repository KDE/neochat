import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.12 as Kirigami

import SortFilterProxyModel 0.2

import NeoChat.Component 2.0
import NeoChat.Component.Timeline 2.0
import org.kde.neochat 0.1

Kirigami.GlobalDrawer {
    id: root
    
    modal: true
    collapsible: true
    collapsed: Kirigami.Settings.isMobile
}
