import QtQuick 2.9
import QtQuick.Controls 2.2

Page {
    property alias lazyLoad: lazyLoadSwitch.checked
    property alias asyncMessageDelegate: asyncMessageDelegateSwitch.checked
    property alias richText: richTextSwitch.checked

    Column {
        Switch {
            id: lazyLoadSwitch
            text: "Lazy load at initial sync"
        }
        Switch {
            id: asyncMessageDelegateSwitch
            text: "Force loading message delegates asynchronously"
        }
        Switch {
            id: richTextSwitch
            text: "Use RichText instead of StyledText"
        }

        Button {
            text: "Invoke GC"
            highlighted: true
            onClicked: gc()
        }
    }
}
