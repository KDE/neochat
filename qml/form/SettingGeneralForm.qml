import QtQuick 2.9
import QtQuick.Controls 2.2
import MatriqueSettings 0.1

Page {
    Column {
        Switch {
            id: lazyLoadSwitch
            text: "Lazy load at initial sync"
            checked: MatriqueSettings.lazyLoad
            onCheckedChanged: MatriqueSettings.lazyLoad = checked
        }
        Switch {
            id: asyncMessageDelegateSwitch
            text: "Force loading message delegates asynchronously"
            checked: MatriqueSettings.asyncMessageDelegate
            onCheckedChanged: MatriqueSettings.asyncMessageDelegate = checked
        }
        Switch {
            id: richTextSwitch
            text: "Use RichText instead of StyledText"
            checked: MatriqueSettings.richText
            onCheckedChanged: MatriqueSettings.richText = checked
        }
        Switch {
            id: pressAndHoldSwitch
            text: "Use press and hold instead of right click"
            checked: MatriqueSettings.pressAndHold
            onCheckedChanged: MatriqueSettings.pressAndHold = checked
        }

        Button {
            text: "Invoke GC"
            highlighted: true
            onClicked: gc()
        }
    }
}
