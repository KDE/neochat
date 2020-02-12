import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.4 as Kirigami

Controls.ItemDelegate {
    default property alias innerObject : column.children

    horizontalPadding: Kirigami.Units.largeSpacing
    verticalPadding: Kirigami.Units.smallSpacing

    contentItem: Column {
        id: column

        SectionDelegate {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(implicitWidth, parent.width)

            visible: showSection
        }
    }
}
