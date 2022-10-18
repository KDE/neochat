// SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>
// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messageformatter.h"

#include <QDomDocument>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlContext>
#include <QQmlProperty>
#include <QTextCursor>
#include <QTextDocumentFragment>

#include <KSyntaxHighlighting/definition.h>
#include <KSyntaxHighlighting/repository.h>
#include <KSyntaxHighlighting/syntaxhighlighter.h>
#include <KSyntaxHighlighting/theme.h>

QTextDocumentFragment copyTextLayoutFrom(QTextDocument *document)
{
    QTextCursor sourceCursor(document);
    sourceCursor.select(QTextCursor::Document);

    QTextDocument helper;

    // copy the content fragment from the source document into our helper document
    QTextCursor curs(&helper);
    curs.insertFragment(sourceCursor.selection());
    curs.select(QTextCursor::Document);

    // not sure why, but fonts get lost. since this is for codeblocks, we can
    // just force the mono font. anyone copying this code would probably want
    // to fix the problem proper if it's not also for codeblocks.
    const auto fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    const int docStart = sourceCursor.selectionStart();
    const int docEnd = helper.characterCount() - 1;

    // since the copied fragment above lost the qsyntaxhighlighter stuff,
    // we gotta go through the qtextlayout and apply those styles to the
    // document
    const auto end = document->findBlock(sourceCursor.selectionEnd()).next();
    for (auto current = document->findBlock(docStart); current.isValid() && current != end; current = current.next()) {
        const auto layout = current.layout();

        // iterate through the formats, applying them to our document
        for (const auto &span : layout->formats()) {
            const int start = current.position() + span.start - docStart;
            const int end = start + span.length;

            curs.setPosition(qMax(start, 0));
            curs.setPosition(qMin(end, docEnd), QTextCursor::KeepAnchor);

            auto fmt = span.format;
            fmt.setFont(fixedFont);

            curs.setCharFormat(fmt);
        }
    }

    return QTextDocumentFragment(&helper);
}

QTextDocumentFragment highlight(const QString &code, const QString &language)
{
    using namespace KSyntaxHighlighting;

    static Repository repo;

    auto theme = repo.themeForPalette(QGuiApplication::palette());
    auto definition = repo.definitionForFileName(QLatin1String("file.") + language);

    QTextDocument doku(code);

    QScopedPointer<SyntaxHighlighter> highlighter(new SyntaxHighlighter(&doku));
    highlighter->setTheme(theme);
    highlighter->setDefinition(definition);

    return copyTextLayoutFrom(&doku);
}

bool extractCodeBlock(QTextCursor cursor, QDomElement element)
{
    const auto codeNode = element.firstChild();
    if (!codeNode.isNull()) {
        const auto code = codeNode.toElement();
        if (!code.isNull() && code.tagName() == QLatin1String("code")) {
            QString lang;
            auto langClass = code.attribute(QLatin1String("class"), QLatin1String("none"));
            if (langClass != QLatin1String("none") && langClass.startsWith(QLatin1String("language-"))) {
                lang = langClass.remove(0, 9);
            }

            if (!lang.isNull()) {
                cursor.insertFragment(highlight(code.text(), lang));
                return true;
            }
        }
    }
    return false;
}

QString MessageFormatter::formatInternal(const QString &messageBody, QTextDocument *document)
{
    QTextCursor curs(document);
    QDomDocument doc(QLatin1String("htmlement"));
    doc.setContent(QStringLiteral("<div>%1</div>").arg(messageBody));
    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (!e.isNull()) {
            if (e.tagName() != QLatin1String("pre") || !extractCodeBlock(curs, e)) {
                QString outText;
                QTextStream out(&outText);
                e.save(out, 0);
                curs.insertHtml(outText);
            }
        }
        n = n.nextSibling();
    }

    Q_EMIT document->contentsChanged();
    return document->toHtml();
}

QString MessageFormatter::format(const QString &messageBody, QQuickTextDocument *doc, QQuickItem *item)
{
    QColor linkColor = QQmlProperty(item, QLatin1String("Kirigami.Theme.linkColor"), qmlContext(item)).read().value<QColor>();

    return formatInternal(messageBody, doc->textDocument());
}
