// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.syntaxhighlighting

import org.kde.neochat

Kirigami.Page {
    property string sourceText

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    title: i18n("Event Source")

    QQC2.ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        QQC2.TextArea {
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

            // opt-out of whatever spell checker a styled TextArea might come with
            Kirigami.SpellCheck.enabled: false

            SyntaxHighlighter {
                textEdit: sourceTextArea
                definition: "JSON"
                repository: Repository
            }
        }
    }
}

