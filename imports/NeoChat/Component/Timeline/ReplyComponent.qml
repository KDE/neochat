/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-or-later
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
        Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
        Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
        Layout.alignment: Qt.AlignTop

        source: replyVisible && reply.author.avatarMediaId ? "image://mxc/" + reply.author.avatarMediaId : ""
        name: replyVisible ? reply.author.displayName : "H"
        color: replyVisible ? reply.author.color : Kirigami.Theme.highlightColor
    }

    ColumnLayout {
        Layout.fillWidth: true

        QQC2.Label {
            Layout.fillWidth: true

            text: replyVisible ? reply.author.displayName : ""
            color: replyVisible ? reply.author.color: null
            wrapMode: Text.Wrap
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: replyVisible ? reply.display : ""
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            textFormat: Text.RichText
        }
    }
}

