// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.syntaxhighlighting 1.0
import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

Kirigami.Page {
    property string sourceText

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    title: i18n("Message Source")

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

        TextArea {
            id: sourceTextArea
            text: sourceText
            readOnly: true
            textFormat: TextEdit.PlainText
            wrapMode: Text.WordWrap
            background: Rectangle {
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false
                color: Kirigami.Theme.backgroundColor
            }

            SyntaxHighlighter {
                textEdit: sourceTextArea
                definition: "JSON"
                repository: Repository
            }
        }
    }
}

