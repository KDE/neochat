import QtQuick 2.12
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.4

import Spectral.Component 2.0
import Spectral.Component.Emoji 2.0
import Spectral.Effect 2.0
import Spectral.Setting 0.1

import Spectral 0.1

import "qrc:/js/md.js" as Markdown

Control {
    property alias isReply: replyItem.visible
    property var replyUser
    property string replyEventID
    property string replyContent

    property alias isAutoCompleting: autoCompleteListView.visible
    property var autoCompleteModel
    property int autoCompleteBeginPosition
    property int autoCompleteEndPosition

    id: root

    padding: 0

    background: Rectangle {
        color: MSettings.darkTheme ? "#303030" : "#fafafa"
        radius: 24

        layer.enabled: true
        layer.effect: ElevationEffect {
            elevation: 2
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 8

            id: replyItem

            visible: false

            spacing: 8

            Avatar {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32

                source: replyUser ? replyUser.avatarUrl : ""
                hint: replyUser ? replyUser.displayName : "No name"
            }

            Label {
                Layout.fillWidth: true

                text: replyContent
                font.pixelSize: 16

                wrapMode: Label.Wrap
            }
        }

        EmojiPicker {
            Layout.fillWidth: true

            id: emojiPicker

            visible: false

            textArea: inputField
            emojiModel: EmojiModel { id: emojiModel }
        }

        ListView {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            Layout.margins: 8

            id: autoCompleteListView

            visible: false

            model: autoCompleteModel

            clip: true
            spacing: 4
            orientation: ListView.Horizontal
            highlightFollowsCurrentItem: true
            keyNavigationWraps: true

            delegate: Control {
                property string autoCompleteText: modelData.displayName || modelData.unicode
                property bool isEmoji: modelData.unicode != null
                readonly property bool highlighted: autoCompleteListView.currentIndex === index

                height: 36
                padding: 8

                background: Rectangle {
                    visible: !isEmoji
                    color: highlighted ? Material.accent : "transparent"
                    border.color: Material.accent
                    border.width: 2
                    radius: height / 2
                }

                contentItem: Row {
                    spacing: 4

                    Text {
                        width: 20
                        height: 20
                        visible: isEmoji
                        text: autoCompleteText
                        font.pixelSize: 24
                        font.family: "Emoji"
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Avatar {
                        width: 20
                        height: 20
                        visible: !isEmoji
                        source: modelData.avatarUrl || null
                    }
                    Label {
                        height: parent.height
                        visible: !isEmoji
                        text: autoCompleteText
                        color: highlighted ? "white" : Material.accent
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        autoCompleteListView.currentIndex = index
                        inputField.replaceAutoComplete(autoCompleteText)
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.leftMargin: 12
            Layout.rightMargin: 12

            visible: emojiPicker.visible || replyItem.visible || autoCompleteListView.visible

            color: MSettings.darkTheme ? "#424242" : "#e7ebeb"
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 0

            ToolButton {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignBottom

                id: uploadButton
                visible: !isReply

                contentItem: MaterialIcon {
                    icon: "\ue226"
                }

                onClicked: currentRoom.chooseAndUploadFile()

                BusyIndicator {
                    anchors.fill: parent

                    running: currentRoom && currentRoom.hasFileUploading
                }
            }

            ToolButton {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignBottom

                id: cancelReplyButton

                visible: isReply

                contentItem: MaterialIcon {
                    icon: "\ue5cd"
                }

                onClicked: clearReply()
            }

            TextArea {
                property real progress: 0

                Layout.fillWidth: true
                Layout.minimumHeight: 48

                id: inputField

                wrapMode: Text.Wrap
                placeholderText: "Send a Message"
                topPadding: 0
                bottomPadding: 0
                selectByMouse: true
                verticalAlignment: TextEdit.AlignVCenter

                text: currentRoom != null ? currentRoom.cachedInput : ""

                background: Item {
                }

                Rectangle {
                    width: currentRoom && currentRoom.hasFileUploading ? parent.width * currentRoom.fileUploadingProgress / 100 : 0
                    height: parent.height

                    opacity: 0.2
                    color: Material.accent
                }

                Timer {
                    id: timeoutTimer

                    repeat: false
                    interval: 2000
                    onTriggered: {
                        repeatTimer.stop()
                        currentRoom.sendTypingNotification(false)
                    }
                }

                Timer {
                    id: repeatTimer

                    repeat: true
                    interval: 5000
                    triggeredOnStart: true
                    onTriggered: currentRoom.sendTypingNotification(true)
                }

                ToolTip {
                    visible: currentRoom
                             && currentRoom.hasUsersTyping
                    text: currentRoom ? currentRoom.usersTyping : ""

                    Material.foreground: "white"
                }

                Keys.onReturnPressed: {
                    if (event.modifiers & Qt.ShiftModifier) {
                        insert(cursorPosition, "\n")
                    } else if (text) {
                        postMessage(text)
                        text = ""
                        closeAll()
                    }
                }

                Keys.onEscapePressed: closeAll()

                Keys.onBacktabPressed: if (isAutoCompleting) autoCompleteListView.decrementCurrentIndex()

                Keys.onTabPressed: {
                    if (isAutoCompleting) {
                        autoCompleteListView.incrementCurrentIndex()
                    } else {
                        autoCompleteBeginPosition = text.substring(0, cursorPosition).lastIndexOf(" ") + 1
                        var autoCompletePrefix = text.substring(0, cursorPosition).split(" ").pop()
                        if (!autoCompletePrefix) return
                        if (autoCompletePrefix.startsWith(":")) {
                            autoCompleteBeginPosition = text.substring(0, cursorPosition).lastIndexOf(" ") + 1
                            autoCompleteModel = emojiModel.filterModel(autoCompletePrefix)
                            if (autoCompleteModel.length === 0) return
                            isAutoCompleting = true
                            autoCompleteEndPosition = cursorPosition
                        } else {
                            autoCompleteModel = currentRoom.getUsers(autoCompletePrefix)
                            if (autoCompleteModel.length === 0) return
                            isAutoCompleting = true
                            autoCompleteEndPosition = cursorPosition
                        }
                    }
                    replaceAutoComplete(autoCompleteListView.currentItem.autoCompleteText)
                }

                onTextChanged: {
                    timeoutTimer.restart()
                    repeatTimer.start()
                    currentRoom.cachedInput = text

                    if (cursorPosition !== autoCompleteBeginPosition && cursorPosition !== autoCompleteEndPosition) {
                        isAutoCompleting = false
                        autoCompleteListView.currentIndex = 0
                    }
                }

                function replaceAutoComplete(word) {
                    remove(autoCompleteBeginPosition, autoCompleteEndPosition)
                    autoCompleteEndPosition = autoCompleteBeginPosition + word.length
                    insert(cursorPosition, word)
                }

                function postMessage(text) {
                    if (text.trim().length === 0) { return }
                    if(!currentRoom) { return }

                    var PREFIX_ME = '/me '
                    var PREFIX_NOTICE = '/notice '
                    var PREFIX_RAINBOW = '/rainbow '
                    var PREFIX_HTML = '/html '
                    var PREFIX_MARKDOWN = '/md '

                    if (isReply) {
                        currentRoom.sendReply(replyUser.id, replyEventID, replyContent, text)
                        return
                    }

                    if (text.indexOf(PREFIX_ME) === 0) {
                        text = text.substr(PREFIX_ME.length)
                        currentRoom.postMessage(text, RoomMessageEvent.Emote)
                        return
                    }
                    if (text.indexOf(PREFIX_NOTICE) === 0) {
                        text = text.substr(PREFIX_NOTICE.length)
                        currentRoom.postMessage(text, RoomMessageEvent.Notice)
                        return
                    }
                    if (text.indexOf(PREFIX_RAINBOW) === 0) {
                        text = text.substr(PREFIX_RAINBOW.length)

                        var parsedText = ""
                        var rainbowColor = ["#ff2b00", "#ff5500", "#ff8000", "#ffaa00", "#ffd500", "#ffff00", "#d4ff00", "#aaff00", "#80ff00", "#55ff00", "#2bff00", "#00ff00", "#00ff2b", "#00ff55", "#00ff80", "#00ffaa", "#00ffd5", "#00ffff", "#00d4ff", "#00aaff", "#007fff", "#0055ff", "#002bff", "#0000ff", "#2a00ff", "#5500ff", "#7f00ff", "#aa00ff", "#d400ff", "#ff00ff", "#ff00d4", "#ff00aa", "#ff0080", "#ff0055", "#ff002b", "#ff0000"]
                        for (var i = 0; i < text.length; i++) {
                            parsedText = parsedText + "<font color='" + rainbowColor[i % rainbowColor.length] + "'>" + text.charAt(i) + "</font>"
                        }
                        currentRoom.postHtmlMessage(text, parsedText, RoomMessageEvent.Text)
                        return
                    }
                    if (text.indexOf(PREFIX_HTML) === 0) {
                        text = text.substr(PREFIX_HTML.length)
                        var re = new RegExp("<.*?>")
                        var plainText = text.replace(re, "")
                        currentRoom.postHtmlMessage(plainText, text, RoomMessageEvent.Text)
                        return
                    }
                    if (text.indexOf(PREFIX_MARKDOWN) === 0) {
                        text = text.substr(PREFIX_MARKDOWN.length)
                        var parsedText = Markdown.markdown_parser(text)
                        currentRoom.postHtmlMessage(text, parsedText, RoomMessageEvent.Text)
                        return
                    }

                    currentRoom.postPlainText(text)
                }
            }

            ToolButton {
                Layout.preferredWidth: 48
                Layout.preferredHeight: 48
                Layout.alignment: Qt.AlignBottom

                id: emojiButton

                contentItem: MaterialIcon {
                    icon: "\ue24e"
                }

                onClicked: emojiPicker.visible = !emojiPicker.visible
            }
        }
    }

    function insert(str) {
        inputField.insert(inputField.cursorPosition, str)
    }

    function clear() {
        inputField.clear()
    }

    function clearReply() {
        isReply = false
        replyUser = null
        replyEventID = ""
        replyContent = ""
    }

    function focus() {
        inputField.forceActiveFocus()
    }

    function closeAll() {
        replyItem.visible = false
        autoCompleteListView.visible = false
        emojiPicker.visible = false
    }
}
