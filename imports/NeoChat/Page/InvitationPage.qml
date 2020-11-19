/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import org.kde.kirigami 2.14 as Kirigami
import QtQuick.Layouts 1.12

Kirigami.Page {
    id: root
    property var room
    title: i18n("Invitation Received - %1", room.displayName)

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        text: i18n("Accept this invitation?")
        RowLayout {
            Button {
                flat: true
                Layout.alignment : Qt.AlignHCenter
                text: i18n("Cancel")

                onClicked: roomManager.getBack();
            }

            Button {
                Layout.alignment : Qt.AlignHCenter
                text: i18n("Reject")
                flat: true

                onClicked: {
                    room.forget()
                    roomManager.getBack();
                }
            }

            Button {
                Layout.alignment : Qt.AlignHCenter
                text: i18n("Accept")

                onClicked: {
                    room.acceptInvitation();
                    roomManager.enterRoom(room);
                }
            }
        }
    }
}

