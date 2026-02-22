// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "chattextitemhelper.h"
#include "chatbartype.h"
#include "richformat.h"

#include <QQuickTextDocument>
#include <QTextCursor>

#include <Kirigami/Platform/PlatformTheme>
#include <qtextdocument.h>

#include "chatbarsyntaxhighlighter.h"
#include "neochatroom.h"

// Because we can't get access to the private header we foward declare this so the SIGNAL() macro works in setTextItem.
namespace QQuickTextEdit
{
enum TextFormat : unsigned int;
}

ChatTextItemHelper::ChatTextItemHelper(QObject *parent)
    : QObject(parent)
    , m_highlighter(new ChatBarSyntaxHighlighter(this))
{
}

NeoChatRoom *ChatTextItemHelper::room() const
{
    if (!m_highlighter) {
        return nullptr;
    }
    return m_highlighter->room;
}

void ChatTextItemHelper::setRoom(NeoChatRoom *room)
{
    if (!m_highlighter) {
        return;
    }
    m_highlighter->room = room;
    Q_EMIT roomChanged();
}

ChatBarType::Type ChatTextItemHelper::type() const
{
    if (!m_highlighter) {
        return ChatBarType::None;
    }
    return m_highlighter->type;
}

void ChatTextItemHelper::setType(ChatBarType::Type type)
{
    if (!m_highlighter) {
        return;
    }
    m_highlighter->type = type;
    Q_EMIT typeChanged();
}

QQuickItem *ChatTextItemHelper::textItem() const
{
    return m_textItem;
}

void ChatTextItemHelper::setTextItem(QQuickItem *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    if (m_textItem) {
        m_textItem->disconnect(this);
        if (const auto textDoc = document()) {
            textDoc->disconnect(this);
        }
    }

    m_textItem = textItem;

    if (m_textItem) {
        connect(m_textItem, SIGNAL(cursorPositionChanged()), this, SLOT(itemCursorPositionChanged()));
        connect(m_textItem, SIGNAL(selectedTextChanged()), this, SLOT(itemSelectedTextChanged()));
        connect(m_textItem, SIGNAL(textFormatChanged(QQuickTextEdit::TextFormat)), this, SLOT(itemTextFormatChanged()));
        if (const auto doc = document()) {
            connect(doc, &QTextDocument::contentsChanged, this, &ChatTextItemHelper::contentsChanged);
            connect(doc, &QTextDocument::contentsChange, this, [this]() {
                m_contentsJustChanged = true;
            });
            connect(doc, &QTextDocument::contentsChange, this, &ChatTextItemHelper::contentsChange);
            m_highlighter->setDocument(doc);
        }
        initialize();
    }

    Q_EMIT textItemChanged();
    Q_EMIT textFormatChanged();
    Q_EMIT charFormatChanged();
    Q_EMIT styleChanged();
    Q_EMIT listChanged();
}

std::optional<Qt::TextFormat> ChatTextItemHelper::textFormat() const
{
    if (!m_textItem) {
        return std::nullopt;
    }
    return static_cast<Qt::TextFormat>(m_textItem->property("textFormat").toInt());
}

void ChatTextItemHelper::itemTextFormatChanged()
{
    Q_EMIT textFormatChanged();
}

QString ChatTextItemHelper::fixedStartChars() const
{
    return m_fixedStartChars;
}

QString ChatTextItemHelper::fixedEndChars() const
{
    return m_fixedEndChars;
    ;
}

void ChatTextItemHelper::setFixedChars(const QString &startChars, const QString &endChars)
{
    if (startChars == m_fixedStartChars && endChars == m_fixedEndChars) {
        return;
    }
    m_fixedStartChars = startChars;
    m_fixedEndChars = endChars;
    initialize();
}

QTextDocumentFragment ChatTextItemHelper::initialFragment() const
{
    return m_initialFragment;
}

void ChatTextItemHelper::setInitialFragment(const QTextDocumentFragment &fragment)
{
    m_initialFragment = fragment;
    initialize();
}

void ChatTextItemHelper::initialize()
{
    const auto doc = document();
    if (!doc) {
        return;
    }

    QTextCursor cursor(doc);
    if (cursor.isNull()) {
        return;
    }

    m_initializingChars = true;

    cursor.beginEditBlock();
    int finalCursorPos = cursor.position();
    if (doc->isEmpty() && !m_initialFragment.isEmpty()) {
        cursor.insertFragment(m_initialFragment);
        finalCursorPos = cursor.position();
    }

    if (!m_fixedStartChars.isEmpty() && doc->characterAt(0) != m_fixedStartChars) {
        cursor.movePosition(QTextCursor::Start);
        cursor.insertText(m_fixedStartChars);
        finalCursorPos += m_fixedStartChars.length();
    }

    if (!m_fixedStartChars.isEmpty() && doc->characterAt(doc->characterCount()) != m_fixedStartChars) {
        cursor.movePosition(QTextCursor::End);
        cursor.keepPositionOnInsert();
        cursor.insertText(m_fixedEndChars);
    }
    setCursorPosition(finalCursorPos);
    cursor.endEditBlock();

    m_initializingChars = false;
}

QTextDocument *ChatTextItemHelper::document() const
{
    if (!m_textItem) {
        return nullptr;
    }
    const auto quickDocument = qvariant_cast<QQuickTextDocument *>(textItem()->property("textDocument"));
    return quickDocument ? quickDocument->textDocument() : nullptr;
}

bool ChatTextItemHelper::isEmpty() const
{
    return markdownText().length() == 0;
}

int ChatTextItemHelper::lineCount() const
{
    if (const auto doc = document()) {
        return doc->lineCount();
    }
    return 0;
}

std::optional<int> ChatTextItemHelper::lineLength(int lineNumber) const
{
    const auto doc = document();
    if (!doc || lineNumber < 0 || lineNumber >= doc->lineCount()) {
        return std::nullopt;
    }
    const auto block = doc->findBlockByLineNumber(lineNumber);
    const auto lineNumInBlock = lineNumber - block.firstLineNumber();
    return block.layout()->lineAt(lineNumInBlock).textLength();
}

QTextDocumentFragment ChatTextItemHelper::takeFirstBlock()
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return {};
    }
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, m_fixedStartChars.length());
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    if (document()->blockCount() <= 1) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, m_fixedEndChars.length());
    }

    const auto block = cursor.selection();
    cursor.removeSelectedText();
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    if (cursor.selectedText() == QChar::ParagraphSeparator) {
        cursor.removeSelectedText();
    }
    cursor.endEditBlock();
    if (document()->characterCount() - 1 <= (m_fixedStartChars.length() + m_fixedEndChars.length())) {
        Q_EMIT cleared(this);
    }
    return block;
}

void ChatTextItemHelper::fillFragments(bool &hasBefore, QTextDocumentFragment &midFragment, std::optional<QTextDocumentFragment> &afterFragment)
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }

    if (cursor.blockNumber() > 0) {
        hasBefore = true;
    }
    auto afterBlock = cursor.blockNumber() < document()->blockCount() - 1;

    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::StartOfBlock);
    if (!hasBefore) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor, m_fixedStartChars.length());
    }
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    if (!afterBlock) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, m_fixedEndChars.length());
    }

    midFragment = cursor.selection();
    if (!midFragment.isEmpty()) {
        cursor.removeSelectedText();
    }

    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    if (cursor.selectedText() == QChar::ParagraphSeparator) {
        cursor.removeSelectedText();
    } else {
        cursor.movePosition(QTextCursor::NextCharacter);
    }

    if (afterBlock) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == QChar::ParagraphSeparator) {
            cursor.removeSelectedText();
        }
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        afterFragment = cursor.selection();
        cursor.removeSelectedText();
    }
    cursor.endEditBlock();
}

void ChatTextItemHelper::insertFragment(const QTextDocumentFragment fragment, InsertPosition position, bool keepPosition)
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }

    int currentPosition;
    switch (position) {
    case Start:
        currentPosition = 0;
        break;
    case End:
        currentPosition = document()->characterCount() - 1;
        break;
    case Cursor:
        currentPosition = cursor.position();
        break;
    }

    if (currentPosition < m_fixedStartChars.length()) {
        currentPosition = m_fixedStartChars.length();
    }
    if (currentPosition >= document()->characterCount() - 1 - m_fixedEndChars.length()) {
        currentPosition = document()->characterCount() - 1 - m_fixedEndChars.length();
    }

    cursor.setPosition(currentPosition);
    if (textFormat() && textFormat() == Qt::PlainText) {
        auto text = fragment.toPlainText();
        text = trim(text);
        cursor.insertText(text);
    } else {
        cursor.insertFragment(fragment);
    }
    if (keepPosition) {
        cursor.setPosition(currentPosition);
    }
    setCursorPosition(cursor.position());
}

std::optional<int> ChatTextItemHelper::cursorPosition() const
{
    if (!m_textItem) {
        return std::nullopt;
    }
    return m_textItem->property("cursorPosition").toInt();
}

QRect ChatTextItemHelper::cursorRectangle() const
{
    if (!m_textItem) {
        return {};
    }
    return m_textItem->property("cursorRectangle").toRect();
}

int ChatTextItemHelper::selectionStart() const
{
    if (!m_textItem) {
        return -1;
    }
    return m_textItem->property("selectionStart").toInt();
}

int ChatTextItemHelper::selectionEnd() const
{
    if (!m_textItem) {
        return -1;
    }
    return m_textItem->property("selectionEnd").toInt();
}

QTextCursor ChatTextItemHelper::textCursor() const
{
    if (!document()) {
        return QTextCursor();
    }

    QTextCursor cursor = QTextCursor(document());
    if (selectionStart() != selectionEnd()) {
        cursor.setPosition(selectionStart());
        cursor.setPosition(selectionEnd(), QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(*cursorPosition());
    }
    return cursor;
}

void ChatTextItemHelper::setCursorPosition(int pos)
{
    if (!m_textItem) {
        return;
    }
    m_textItem->setProperty("cursorPosition", pos);
}

void ChatTextItemHelper::setSelection(int selectionStart, int selectionEnd)
{
    if (!m_textItem) {
        return;
    }
    m_selectionJustChanged = true;
    metaObject()->invokeMethod(m_textItem, "select", Qt::DirectConnection, selectionStart, selectionEnd);
}

void ChatTextItemHelper::setCursorVisible(bool visible)
{
    if (!m_textItem) {
        return;
    }
    m_textItem->setProperty("cursorVisible", visible);
}

void ChatTextItemHelper::setCursorFromTextItem(ChatTextItemHelper *textItem, bool infront)
{
    const auto doc = document();
    if (!doc) {
        return;
    }

    m_textItem->forceActiveFocus();

    if (!textItem) {
        const auto docLastBlockLayout = doc->lastBlock().layout();
        setCursorPosition(infront ? 0 : docLastBlockLayout->lineAt(docLastBlockLayout->lineCount() - 1).textStart());
        setCursorVisible(true);
        return;
    }

    const auto previousLinePosition = textItem->textCursor().positionInBlock();
    const auto newMaxLineLength = lineLength(infront ? 0 : lineCount() - 1);
    setCursorPosition(std::min(previousLinePosition, newMaxLineLength ? *newMaxLineLength : 0) + (infront ? 0 : doc->lastBlock().position()));
    setCursorVisible(true);
}

void ChatTextItemHelper::itemCursorPositionChanged()
{
    if (m_initializingChars) {
        return;
    }
    const auto currentCursorPosition = cursorPosition();
    if (!currentCursorPosition) {
        return;
    }
    if (*currentCursorPosition < m_fixedStartChars.length() || *currentCursorPosition > document()->characterCount() - 1 - m_fixedEndChars.length()) {
        setCursorPosition(
            std::min(std::max(*currentCursorPosition, int(m_fixedStartChars.length())), int(document()->characterCount() - 1 - m_fixedEndChars.length())));
        return;
    }

    Q_EMIT cursorPositionChanged(m_contentsJustChanged);
    m_contentsJustChanged = false;
    Q_EMIT textFormatChanged();
    Q_EMIT charFormatChanged();
    Q_EMIT styleChanged();
    Q_EMIT listChanged();
}

void ChatTextItemHelper::itemSelectedTextChanged()
{
    if (m_selectionJustChanged) {
        m_selectionJustChanged = false;
        return;
    }
    Q_EMIT selectedTextChanged();
}

void ChatTextItemHelper::mergeFormatOnCursor(RichFormat::Format format, QTextCursor cursor)
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return;
        }
    }
    switch (RichFormat::typeForFormat(format)) {
    case RichFormat::Text:
        mergeTextFormatOnCursor(format, cursor);
        return;
    case RichFormat::List:
        mergeListFormatOnCursor(format, cursor);
        return;
    case RichFormat::Block:
        if (format != RichFormat::Paragraph) {
            return;
        }
    case RichFormat::Style:
        mergeStyleFormatOnCursor(format, cursor);
        return;
    default:
        return;
    }
}

void ChatTextItemHelper::mergeTextFormatOnCursor(RichFormat::Format format, QTextCursor cursor)
{
    if (RichFormat::typeForFormat(format) != RichFormat::Text) {
        return;
    }

    const auto theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
    const auto charFormat = RichFormat::charFormatForFormat(format, RichFormat::hasFormat(cursor, format), theme->alternateBackgroundColor());
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    if (!cursor.hasSelection() && isEmpty()) {
        cursor.mergeBlockCharFormat(charFormat);
    } else if (!cursor.hasSelection()) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(charFormat);
    } else {
        cursor.mergeCharFormat(charFormat);
    }
    Q_EMIT charFormatChanged();
}

void ChatTextItemHelper::mergeStyleFormatOnCursor(RichFormat::Format format, QTextCursor cursor)
{
    // Paragraph is special because it is normally a Block format but if we're already
    // in a Paragraph it clears any existing style.
    if (!(RichFormat::typeForFormat(format) == RichFormat::Style || format == RichFormat::Paragraph)) {
        return;
    }

    cursor.beginEditBlock();
    cursor.mergeBlockFormat(RichFormat::blockFormatForFormat(format));

    // Applying style to the current line or selection
    QTextCursor selectCursor = cursor;
    if (selectCursor.hasSelection()) {
        QTextCursor top = selectCursor;
        top.setPosition(qMin(top.anchor(), top.position()));
        top.movePosition(QTextCursor::StartOfBlock);

        QTextCursor bottom = selectCursor;
        bottom.setPosition(qMax(bottom.anchor(), bottom.position()));
        bottom.movePosition(QTextCursor::EndOfBlock);

        selectCursor.setPosition(top.position(), QTextCursor::MoveAnchor);
        selectCursor.setPosition(bottom.position(), QTextCursor::KeepAnchor);
    } else {
        selectCursor.select(QTextCursor::BlockUnderCursor);
    }

    const auto chrfmt = RichFormat::charFormatForFormat(format);
    selectCursor.mergeCharFormat(chrfmt);
    cursor.mergeBlockCharFormat(chrfmt);
    cursor.endEditBlock();

    Q_EMIT styleChanged();
}

void ChatTextItemHelper::mergeListFormatOnCursor(RichFormat::Format format, const QTextCursor &cursor)
{
    m_nestedListHelper.handleOnBulletType(RichFormat::listStyleForFormat(format), cursor);
    Q_EMIT listChanged();
}

bool ChatTextItemHelper::canIndentListMoreAtCursor(QTextCursor cursor) const
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return false;
        }
    }
    return m_nestedListHelper.canIndent(cursor) && cursor.blockFormat().headingLevel() == 0;
}

bool ChatTextItemHelper::canIndentListLessAtCursor(QTextCursor cursor) const
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return false;
        }
    }
    return m_nestedListHelper.canDedent(cursor) && cursor.blockFormat().headingLevel() == 0;
}

void ChatTextItemHelper::indentListMoreAtCursor(QTextCursor cursor)
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return;
        }
    }
    m_nestedListHelper.handleOnIndentMore(cursor);
    Q_EMIT listChanged();
}

void ChatTextItemHelper::indentListLessAtCursor(QTextCursor cursor)
{
    if (cursor.isNull()) {
        cursor = textCursor();
        if (cursor.isNull()) {
            return;
        }
    }
    m_nestedListHelper.handleOnIndentLess(cursor);
    Q_EMIT listChanged();
}

void ChatTextItemHelper::forceActiveFocus() const
{
    if (!m_textItem) {
        return;
    }
    m_textItem->forceActiveFocus();
}

void ChatTextItemHelper::rehighlight() const
{
    m_highlighter->rehighlight();
}

bool ChatTextItemHelper::hasRichFormatting() const
{
    return markdownText() != plainText();
}

QString ChatTextItemHelper::markdownText() const
{
    const auto doc = document();
    if (!doc) {
        return {};
    }
    return trim(doc->toMarkdown(QTextDocument::MarkdownDialectGitHub));
}

QString ChatTextItemHelper::plainText() const
{
    const auto doc = document();
    if (!doc) {
        return {};
    }
    return trim(doc->toPlainText());
}

QTextDocumentFragment ChatTextItemHelper::toFragment() const
{
    auto cursor = textCursor();
    if (cursor.isNull()) {
        return {};
    }
    cursor.select(QTextCursor::Document);
    return cursor.selection();
}

QString ChatTextItemHelper::trim(QString string) const
{
    while (string.startsWith(u"\n"_s)) {
        string.removeFirst();
    }
    while (string.endsWith(u"\n"_s)) {
        string.removeLast();
    }
    return string;
}

#include "moc_chattextitemhelper.cpp"
