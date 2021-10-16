// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.syntaxhighlighting 1.0
import org.kde.kirigami 2.15 as Kirigami

Kirigami.OverlaySheet {

    property string sourceText

    title: i18n("Message Source")

    TextArea {
        id: sourceTextArea
        text: sourceText
        readOnly: true
        wrapMode: Text.WordWrap

        SyntaxHighlighter {
            textEdit: sourceTextArea
            repository: Repository
            definition: "JSON"
        }
    }
}

