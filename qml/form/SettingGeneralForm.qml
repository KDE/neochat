import QtQuick 2.9
import QtQuick.Controls 2.2

Page {
    property alias lazyLoad: lazyLoadSwitch.checked

    Column {
        Switch {
            id: lazyLoadSwitch
            text: "Lazy Load at Initial Sync"
        }
    }
}
