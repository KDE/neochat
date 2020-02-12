import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.4 as Kirigami

Item {
    default property alias innerObject : column.children

    readonly property int horizontalPadding: Kirigami.Units.largeSpacing
    readonly property int verticalPadding: Kirigami.Units.smallSpacing

    height: column.implicitHeight + verticalPadding * 2

    ColumnLayout {
        x: horizontalPadding
        y: verticalPadding

        width: parent.width - horizontalPadding * 2

        id: column

        SectionDelegate {
            Layout.maximumWidth: parent.width
            Layout.alignment: Qt.AlignHCenter

            visible: showSection
        }
    }

    Controls.ItemDelegate {
        anchors.fill: parent

        z: -1
    }
}
