/* SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.14 as Kirigami

Loader {
    id: root
    property bool isEdit: false
    property var user: null
    property string content: ""
    property string avatarMediaUrl: user ? "image://mxc/" + replyUser.avatarMediaId : ""

    signal clearEditReplyTriggered()

    active: visible
    sourceComponent: Pane {
        id: replyPane

        Kirigami.Theme.colorSet: Kirigami.Theme.View

        spacing: leftPadding

        contentItem: RowLayout {
            Layout.fillWidth: true
            spacing: replyPane.spacing

            FontMetrics {
                id: fontMetrics
                font: textArea.font
            }

            Kirigami.Avatar {
                id: avatar
                Layout.alignment: textContentLayout.height > avatar.height ? Qt.AlignHCenter | Qt.AlignTop : Qt.AlignCenter
                Layout.preferredWidth: Layout.preferredHeight
                Layout.preferredHeight: fontMetrics.lineSpacing * 2 - fontMetrics.leading
                source: root.avatarMediaUrl
                name: user ? user.displayName : ""
                color: user ? user.color : "transparent"
                visible: Boolean(user)
            }

            ColumnLayout {
                id: textContentLayout
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: true
                spacing: fontMetrics.leading
                Label {
                    textFormat: TextEdit.RichText
                    wrapMode: Label.Wrap
                    text: {
                        let stylesheet = "<style>.user-pill{}</style>"
                        let heading = "<b>%1</b>"
                        let userName = user ? "<font color=\""+ user.color +"\">" + user.displayName + "</font>" : ""
                        if (isEdit) {
                            heading = heading.arg(i18n("Editing message:")) + "<br/>"
                        } else {
                            heading = heading.arg(i18n("Replying to %1:"))
                            heading = heading.arg(userName) + "<br/>"
                        }

                        return stylesheet + heading
                    }
                }
                ScrollView {
                    Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                    Layout.fillWidth: true
                    Layout.maximumHeight: fontMetrics.lineSpacing * 8 - fontMetrics.leading
                    TextArea {
                        id: textArea
                        leftPadding: 0
                        rightPadding: 0
                        topPadding: 0
                        bottomPadding: 0
                        text: {
                            let stylesheet = "<style> a{color:"+Kirigami.Theme.linkColor+";}.user-pill{}</style>"
                            let userName = user ? "<font color=\""+ user.color +"\">" + user.displayName + "</font>" : ""
                            return stylesheet + content
                        }
                        selectByMouse: true
                        selectByKeyboard: true
                        readOnly: true
                        wrapMode: Label.Wrap
                        textFormat: TextEdit.RichText
                        background: null
                        HoverHandler {
                            cursorShape: textArea.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                        }
                    }
                }
            }

            Button {
                id: cancelReplyButton
                Layout.alignment: avatar.Layout.alignment
                icon.name: "dialog-cancel"
                text: i18n("Cancel")
                display: AbstractButton.IconOnly
                onClicked: {
                    clearEditReplyTriggered()
                }
                ToolTip.text: text
                ToolTip.visible: hovered
            }
        }

        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
        }
    }
}
