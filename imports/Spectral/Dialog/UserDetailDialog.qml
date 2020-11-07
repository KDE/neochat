import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import Spectral.Component 2.0
import Spectral.Effect 2.0
import Spectral.Setting 0.1

Dialog {
    property var room
    property var user

    property string displayName: user.displayName
    property string avatarMediaId: user.avatarMediaId
    property string avatarUrl: user.avatarUrl

    anchors.centerIn: parent
    width: 360

    id: root

    modal: true

    contentItem: ColumnLayout {
        RowLayout {
            Layout.fillWidth: true

            spacing: 16

            Kirigami.Avatar {
                Layout.preferredWidth: 72
                Layout.preferredHeight: 72

                name: displayName
                source: avatarMediaId ? "image://mxc/" + avatarMediaId : ""

                RippleEffect {
                    anchors.fill: parent

                    circular: true

                    onPrimaryClicked: {
                        if (avatarMediaId) {
                            fullScreenImage.createObject(parent, {"filename": displayName, "localPath": room.urlToMxcUrl(avatarUrl)}).showFullScreen()
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true

                    font.pixelSize: 18
                    font.bold: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: displayName
                    color: MPalette.foreground
                }

                Label {
                    Layout.fillWidth: true

                    text: "Online"
                    color: MPalette.lighter
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true

            spacing: 8

            ColumnLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: user.id
                    color: MPalette.accent
                }

                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: "User ID"
                    color: MPalette.lighter
                }
            }
        }

        MenuSeparator {
            Layout.fillWidth: true
        }

        Control {
            Layout.fillWidth: true

            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: room.connection.isIgnored(user) ? "Unignore this user" : "Ignore this user"

                    color: MPalette.accent
                }
            }

            background: RippleEffect {
                onPrimaryClicked: {
                    root.close()
                    room.connection.isIgnored(user) ? room.connection.removeFromIgnoredUsers(user) : room.connection.addToIgnoredUsers(user)
                }
            }
        }

        Control {
            Layout.fillWidth: true

            contentItem: RowLayout {
                Label {
                    Layout.fillWidth: true

                    wrapMode: Label.Wrap
                    text: "Kick this user"

                    color: MPalette.accent
                }
            }

            background: RippleEffect {
                onPrimaryClicked: room.kickMember(user.id)
            }
        }
    }

    Component {
        id: fullScreenImage

        FullScreenImage {}
    }

    onClosed: destroy()
}

