/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

RowLayout {
    Layout.fillWidth: true

    visible: replyVisible

    Rectangle {
        Layout.preferredWidth: Kirigami.Units.smallSpacing
        Layout.fillHeight: true

        color: Kirigami.Theme.highlightColor
    }

    Kirigami.Avatar {
        Layout.preferredWidth: Kirigami.Units.gridUnit
        Layout.preferredHeight: Kirigami.Units.gridUnit
        Layout.alignment: Qt.AlignTop

        source: replyVisible && reply.author.avatarMediaId ? "image://mxc/" + reply.author.avatarMediaId : ""
        name: replyVisible ? reply.author.displayName : "H"
        color: replyVisible ? reply.author.color : Kirigami.Theme.highlightColor
    }

    ColumnLayout {
        id: replyLayout
        Layout.fillWidth: true

        QQC2.Label {
            Layout.fillWidth: true
            text: replyVisible ? reply.author.displayName : ""
            color: replyVisible ? reply.author.color: null
            elide: Text.ElideRight
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: replyVisible ? reply.display : ""
            textFormat: Text.RichText
            elide: Text.ElideRight
        }
    }
}

