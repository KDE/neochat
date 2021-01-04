/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Qt.labs.platform 1.0 as Platform
import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Component 1.0
import NeoChat.Component.Emoji 1.0
import NeoChat.Dialog 1.0
import NeoChat.Page 1.0

import org.kde.neochat 1.0

ToolBar {
    id: root

    property alias isReply: replyItem.visible
    property bool isReaction: false
    property var replyUser
    property string replyEventID: ""
    property string replyContent: ""

    property string editEventId

    property alias isAutoCompleting: autoCompleteListView.visible
    property var autoCompleteModel
    property int autoCompleteBeginPosition
    property int autoCompleteEndPosition

    property bool hasAttachment: false
    property url attachmentPath: ""
    property var attachmentMimetype: FileType.mimeTypeForUrl(attachmentPath)
    property bool hasImageAttachment: hasAttachment && attachmentMimetype.valid
        && FileType.supportedImageFormats.includes(attachmentMimetype.preferredSuffix)

    position: ToolBar.Footer

    function addText(text) {
        inputField.insert(inputField.length, text)
    }
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    Action {
        id: pasteAction
        shortcut: StandardKey.Paste
        onTriggered: {
            if (Clipboard.hasImage) {
                root.pasteImage();
            }
            activeFocusItem.paste();
        }
    }

    contentItem: ColumnLayout {
        id: layout
        spacing: 0
        EmojiPicker {
            id: emojiPicker

            Layout.fillWidth: true

            visible: false

            textArea: inputField
            emojiModel: EmojiModel { id: emojiModel }
            onChosen: {
                textArea.insert(textArea.cursorPosition, emoji);
                textArea.forceActiveFocus();
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 8

            id: replyItem

            visible: false

            spacing: 8

            Control {
                Layout.alignment: Qt.AlignTop

                padding: 4

                contentItem: RowLayout {
                    Kirigami.Avatar {
                        Layout.preferredWidth: Kirigami.Units.gridUnit
                        Layout.preferredHeight: Kirigami.Units.gridUnit

                        source: replyUser ? ("image://mxc/" + replyUser.avatarMediaId) : ""
                        name: replyUser ? replyUser.displayName : i18n("No name")
                    }

                    Label {
                        Layout.alignment: Qt.AlignVCenter
                        text: replyUser ? replyUser.displayName : i18n("No name")
                        rightPadding: 8
                    }
                }
            }

            TextEdit {
                Layout.fillWidth: true

                text: "<style>a{color: " + color + ";} .user-pill{}</style>" + replyContent
                color: Kirigami.Theme.textColor

                selectByMouse: true
                readOnly: true
                wrapMode: Label.Wrap
                textFormat: Text.RichText
                selectedTextColor: "white"
            }
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
                readonly property string displayText: modelData.displayName ?? modelData.unicode
                readonly property bool isEmoji: modelData.unicode != null
                readonly property bool highlighted: autoCompleteListView.currentIndex == index

                padding: Kirigami.Units.smallSpacing

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Label {
                        width: Kirigami.Units.gridUnit
                        height: Kirigami.Units.gridUnit
                        visible: isEmoji
                        text: displayText
                        font.family: "Emoji"
                        font.pointSize: 20
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Kirigami.Avatar {
                        Layout.preferredWidth: Kirigami.Units.gridUnit
                        Layout.preferredHeight: Kirigami.Units.gridUnit
                        source: modelData.avatarMediaId ? ("image://mxc/" + modelData.avatarMediaId) : ""
                        color: modelData.color ? Qt.darker(modelData.color, 1.1) : null
                        visible: !isEmoji
                    }
                    Label {
                        Layout.fillHeight: true

                        visible: !isEmoji
                        text: displayText
                        color: highlighted ? Kirigami.Theme.highlightTextColor : Kirigami.Theme.textColor
                        font.underline: highlighted
                        verticalAlignment: Text.AlignVCenter
                        rightPadding: Kirigami.Units.largeSpacing
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        autoCompleteListView.currentIndex = index
                        inputField.autoComplete();
                    }
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            visible: emojiPicker.visible || replyItem.visible || autoCompleteListView.visible
        }

        Image {
            Layout.preferredHeight: Kirigami.Units.gridUnit * 10
            source: attachmentPath
            visible: hasImageAttachment
            fillMode: Image.PreserveAspectFit
            Layout.preferredWidth: paintedWidth
            RowLayout {
                anchors.right: parent.right
                Button {
                    icon.name: "document-edit"

                    // HACK: Use a component because an url doesn't work
                    Component {
                        id: imageEditorPage
                        ImageEditorPage {
                            imagePath: attachmentPath
                        }
                    }
                    onClicked: {
                        let imageEditor = applicationWindow().pageStack.layers.push(imageEditorPage, {
                            imagePath: attachmentPath
                        });
                        imageEditor.newPathChanged.connect(function(newPath) {
                            applicationWindow().pageStack.layers.pop();
                            attachmentPath = newPath;
                        });
                    }
                    ToolTip {
                        text: i18n("Edit")
                    }
                }
                Button {
                    icon.name: "dialog-cancel"
                    onClicked: {
                        hasAttachment = false;
                        attachmentPath = "";
                    }
                    ToolTip {
                        text: i18n("Cancel")
                    }
                }
            }
            Rectangle {
                color: Qt.rgba(255, 255, 255, 40)
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                implicitHeight: fileLabel.implicitHeight

                Label {
                    id: fileLabel
                    Layout.alignment: Qt.AlignVCenter
                    text: attachmentPath !== "" ? attachmentPath.toString().substring(attachmentPath.toString().lastIndexOf('/') + 1, attachmentPath.length) : ""
                }
            }
        }

        RowLayout {
            visible: hasAttachment && !hasImageAttachment
            ToolButton {
                icon.name: "dialog-cancel"
                onClicked: {
                    hasAttachment = false;
                    attachmentPath = "";
                }
            }

            Kirigami.Icon {
                id: mimetypeIcon
                implicitHeight: Kirigami.Units.fontMetrics.roundedIconSize(horizontalFileLabel.implicitHeight)
                implicitWidth: implicitHeight
                source: attachmentMimetype.iconName
            }

            Label {
                id: horizontalFileLabel
                Layout.alignment: Qt.AlignVCenter
                text: attachmentPath !== "" ? attachmentPath.toString().substring(attachmentPath.toString().lastIndexOf('/') + 1, attachmentPath.length) : ""
            }
        }

        RowLayout {
            visible: editEventId.length > 0
            ToolButton {
                icon.name: "dialog-cancel"
                onClicked: clearEditReply();
            }

            Label {
                Layout.alignment: Qt.AlignVCenter
                text: i18n("Edit Message")
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            visible: hasAttachment
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 0 //Kirigami.Units.smallSpacing

            Button {
                id: cancelReplyButton

                visible: isReply

                icon.name: "dialog-cancel"

                onClicked: clearEditReply()
            }


            ScrollView {
                Layout.fillWidth: true
                Layout.maximumHeight: inputField.lineHeight * 8
                TextArea {
                    id: inputField
                    property real progress: 0
                    property bool autoAppeared: false

                    ChatDocumentHandler {
                        id: documentHandler
                        document: inputField.textDocument
                        cursorPosition: inputField.cursorPosition
                        selectionStart: inputField.selectionStart
                        selectionEnd: inputField.selectionEnd
                        room: currentRoom ?? null
                    }


                    property int lineHeight: contentHeight / lineCount

                    wrapMode: Text.Wrap
                    placeholderText: i18n("Write your message...")
                    topPadding: 0
                    bottomPadding: 0
                    leftPadding: Kirigami.Units.smallSpacing
                    selectByMouse: true
                    verticalAlignment: TextEdit.AlignVCenter

                    text: currentRoom != null ? currentRoom.cachedInput : ""

                    background: MouseArea {
                        acceptedButtons: Qt.NoButton
                        cursorShape: Qt.IBeamCursor
                        z: 1
                    }

                    Rectangle {
                        width: currentRoom && currentRoom.hasFileUploading ? parent.width * currentRoom.fileUploadingProgress / 100 : 0
                        height: parent.height

                        opacity: 0.2
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

                    Keys.onReturnPressed: {
                        if (isAutoCompleting) {
                            inputField.autoComplete();

                            isAutoCompleting = false;
                            return;
                        }
                        if (event.modifiers & Qt.ShiftModifier) {
                            insert(cursorPosition, "\n")
                        } else {
                            postMessage()
                            text = ""
                            clearEditReply()
                            closeAll()
                        }
                    }

                    Keys.onEscapePressed: {
                        clearEditReply();
                        closeAll();
                    }

                    Keys.onPressed: {
                        if (event.key === Qt.Key_PageDown) {
                            switchRoomDown();
                        } else if (event.key === Qt.Key_PageUp) {
                            switchRoomUp();
                        } else if (event.key === Qt.Key_V && event.modifiers & Qt.ControlModifier) {
                            root.pasteImage();
                        }
                    }

                    Keys.onBacktabPressed: {
                        if (event.modifiers & Qt.ControlModifier) {
                            switchRoomUp();
                            return;
                        }
                        if (isAutoCompleting) {
                            autoCompleteListView.decrementCurrentIndex();
                        }
                    }

                    Keys.onTabPressed: {
                        if (event.modifiers & Qt.ControlModifier) {
                            switchRoomDown();
                            return;
                        }
                        if (!isAutoCompleting) {
                            return;
                        }

                        // TODO detect moved cursor

                        // ignore first time tab was clicked so that user can select
                        // first emoji/user
                        if (autoAppeared === false) {
                            autoCompleteListView.incrementCurrentIndex()
                        } else {
                            autoAppeared = false;
                        }

                        inputField.autoComplete();
                    }

                    onTextChanged: {
                        timeoutTimer.restart()
                        repeatTimer.start()
                        currentRoom.cachedInput = text
                        autoAppeared = false;

                        const autocompletionInfo = documentHandler.getAutocompletionInfo();

                        if (autocompletionInfo.type === ChatDocumentHandler.Ignore) {
                            return;
                        }
                        if (autocompletionInfo.type === ChatDocumentHandler.None) {
                            isAutoCompleting = false;
                            autoCompleteListView.currentIndex = 0;
                            return;
                        }

                        if (autocompletionInfo.type === ChatDocumentHandler.User) {
                            autoCompleteModel = currentRoom.getUsers(autocompletionInfo.keyword);
                        } else {
                            autoCompleteModel = emojiModel.filterModel(autocompletionInfo.keyword);
                        }

                        if (autoCompleteModel.length === 0) {
                            isAutoCompleting = false;
                            autoCompleteListView.currentIndex = 0;
                            return;
                        }
                        isAutoCompleting = true
                        autoAppeared = true;
                        autoCompleteEndPosition = cursorPosition
                    }

                    function postMessage() {
                        // Qt wraps lines so we need to use a small hack
                        // to remove the wrapped lines but not break the empty
                        // lines.
                        const updatedText = inputField.text.trim()
                            .replace(/@([^: ]*):([^ ]*\.[^ ]*)/, "[@$1:$2](https://matrix.to/#/@$1:$2)");
                        documentHandler.postMessage(updatedText, attachmentPath, replyEventID, editEventId);
                        clearAttachment();
                        currentRoom.markAllMessagesAsRead();
                        clear();
                        text = Qt.binding(function() {
                            return currentRoom != null ? currentRoom.cachedInput : "";
                        });
                    }

                    function autoComplete() {
                        documentHandler.replaceAutoComplete(autoCompleteListView.currentItem.displayText)
                    }
                }
            }


            ToolButton {
                id: emojiButton
                icon.name: "preferences-desktop-emoticons"
                icon.color: "transparent"

                checkable: true
                checked: emojiPicker.visible
                onToggled: emojiPicker.visible = !emojiPicker.visible

                ToolTip {
                    text: i18n("Add an Emoji")
                }
            }

            ToolButton {
                id: uploadButton

                visible: !isReply && !hasAttachment

                icon.name: "mail-attachment"

                onClicked: {
                    if (Clipboard.hasImage) {
                        attachDialog.open()
                    } else {
                        var fileDialog = openFileDialog.createObject(ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function(path) {
                            if (!path) return

                            root.attach(path)
                        })

                        fileDialog.open()
                    }
                }

                ToolTip {
                    text: i18n("Attach an image or file")
                }

                BusyIndicator {
                    anchors.fill: parent

                    running: currentRoom && currentRoom.hasFileUploading
                }
            }

            ToolButton {
                icon.name: "document-send"
                icon.color: "transparent"

                onClicked: {
                    inputField.postMessage()
                    inputField.text = ""
                    root.clearEditReply()
                    root.closeAll()
                }

                ToolTip {
                    text: i18n("Send message")
                }
            }
        }
    }

    background: Rectangle {
        implicitHeight: 40
        color: Kirigami.Theme.backgroundColor
        Kirigami.Separator {
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
        }
    }


    function insert(str) {
        inputField.insert(inputField.cursorPosition, str)
    }

    function clear() {
        inputField.clear()
    }

    function clearEditReply() {
        isReply = false;
        replyUser = null;
        root.replyContent = "";
        root.replyEventID = "";
        root.editEventId = "";
    }

    function focus() {
        inputField.forceActiveFocus()
    }

    function edit(editContent, editEventId) {
        inputField.text = editContent;
        root.editEventId = editEventId
    }

    function closeAll() {
        replyItem.visible = false
        autoCompleteListView.visible = false
        emojiPicker.visible = false
    }

    function attach(localPath) {
        hasAttachment = true
        attachmentPath = localPath
    }

    function clearAttachment() {
        hasAttachment = false
        attachmentPath = ""
    }

    function pasteImage() {
        var localPath = Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png";
        if (!Clipboard.saveImage(localPath)) {
            return;
        }
        root.attach(localPath);
    }
}
