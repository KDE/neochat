// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chatdocumenthandler.h"

#include <QQmlFile>
#include <QQmlFileSelector>
#include <QQuickTextDocument>
#include <QStringBuilder>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextList>
#include <QTextTable>
#include <QTimer>

#include <Kirigami/Platform/PlatformTheme>
#include <KColorScheme>

#include <Sonnet/BackgroundChecker>
#include <Sonnet/Settings>
#include <qfont.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qtextcursor.h>
#include <sched.h>

#include "chatbartype.h"
#include "chatdocumenthandler_logging.h"
#include "chatmarkdownhelper.h"
#include "eventhandler.h"

using namespace Qt::StringLiterals;

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
    QPointer<NeoChatRoom> room;
    QTextCharFormat mentionFormat;
    QTextCharFormat errorFormat;
    Sonnet::BackgroundChecker checker;
    Sonnet::Settings settings;
    QList<QPair<int, QString>> errors;
    QString previousText;
    QTimer rehighlightTimer;
    SyntaxHighlighter(QObject *parent)
        : QSyntaxHighlighter(parent)
    {
        m_theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
        connect(m_theme, &Kirigami::Platform::PlatformTheme::colorsChanged, this, [this]() {
            mentionFormat.setForeground(m_theme->linkColor());
            errorFormat.setForeground(m_theme->negativeTextColor());
        });

        mentionFormat.setFontWeight(QFont::Bold);
        mentionFormat.setForeground(m_theme->linkColor());

        errorFormat.setForeground(m_theme->negativeTextColor());
        errorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

        connect(&checker, &Sonnet::BackgroundChecker::misspelling, this, [this](const QString &word, int start) {
            errors += {start, word};
            checker.continueChecking();
        });
        connect(&checker, &Sonnet::BackgroundChecker::done, this, [this]() {
            rehighlightTimer.start();
        });
        rehighlightTimer.setInterval(100);
        rehighlightTimer.setSingleShot(true);
        rehighlightTimer.callOnTimeout(this, &QSyntaxHighlighter::rehighlight);
    }
    void highlightBlock(const QString &text) override
    {
        if (settings.checkerEnabledByDefault()) {
            if (text != previousText) {
                previousText = text;
                checker.stop();
                errors.clear();
                checker.setText(text);
            }
            for (const auto &error : errors) {
                setFormat(error.first, error.second.size(), errorFormat);
            }
        }
        auto handler = dynamic_cast<ChatDocumentHandler *>(parent());
        auto room = handler->room();
        if (!room) {
            return;
        }
        if (!room) {
            return;
        }
        auto mentions = room->cacheForType(handler->type())->mentions();
        mentions->erase(std::remove_if(mentions->begin(),
                                       mentions->end(),
                                       [this](auto &mention) {
                                           if (document()->toPlainText().isEmpty()) {
                                               return false;
                                           }

                                           if (mention.cursor.position() == 0 && mention.cursor.anchor() == 0) {
                                               return true;
                                           }

                                           if (mention.cursor.position() - mention.cursor.anchor() != mention.text.size()) {
                                               mention.cursor.setPosition(mention.start);
                                               mention.cursor.setPosition(mention.cursor.anchor() + mention.text.size(), QTextCursor::KeepAnchor);
                                           }

                                           if (mention.cursor.selectedText() != mention.text) {
                                               return true;
                                           }
                                           if (currentBlock() == mention.cursor.block()) {
                                               mention.start = mention.cursor.anchor();
                                               mention.position = mention.cursor.position();
                                               setFormat(mention.cursor.selectionStart(), mention.cursor.selectedText().size(), mentionFormat);
                                           }
                                           return false;
                                       }),
                        mentions->end());
    }

private:
    Kirigami::Platform::PlatformTheme *m_theme = nullptr;
};

ChatDocumentHandler::ChatDocumentHandler(QObject *parent)
    : QObject(parent)
    , m_highlighter(new SyntaxHighlighter(this))
    , m_completionModel(new CompletionModel(this))
{
    m_markdownHelper = new ChatMarkdownHelper(this);
    connect(this, &ChatDocumentHandler::formatChanged, m_markdownHelper, &ChatMarkdownHelper::handleExternalFormatChange);
}

int ChatDocumentHandler::completionStartIndex() const
{
    const qsizetype cursor = cursorPosition();
    const auto &text = getText();

    auto start = std::min(cursor, text.size()) - 1;
    while (start > -1) {
        if (text.at(start) == QLatin1Char(' ')) {
            start++;
            break;
        }
        start--;
    }
    return start;
}

ChatBarType::Type ChatDocumentHandler::type() const
{
    return m_type;
}

void ChatDocumentHandler::setType(ChatBarType::Type type)
{
    if (type == m_type) {
        return;
    }
    m_type = type;
    Q_EMIT typeChanged();
}

NeoChatRoom *ChatDocumentHandler::room() const
{
    return m_room;
}

void ChatDocumentHandler::setRoom(NeoChatRoom *room)
{
    if (m_room == room) {
        return;
    }

    m_room = room;
    m_completionModel->setRoom(m_room);
    Q_EMIT roomChanged();
}

QQuickItem *ChatDocumentHandler::textItem() const
{
    return m_textItem;
}

void ChatDocumentHandler::setTextItem(QQuickItem *textItem)
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

    m_highlighter->setDocument(document());
    if (m_textItem) {
        connect(m_textItem, SIGNAL(cursorPositionChanged()), this, SLOT(updateCursor()));
        if (document()) {
            connect(document(), &QTextDocument::contentsChanged, this, &ChatDocumentHandler::contentsChanged);
            connect(document(), &QTextDocument::contentsChanged, this, &ChatDocumentHandler::updateCursor);
            connect(document(), &QTextDocument::contentsChange, this, [this](int position) {
                auto cursor = textCursor();
                if (cursor.isNull()) {
                    return;
                }
                cursor.setPosition(position);
                cursor.select(QTextCursor::WordUnderCursor);
                if (!cursor.selectedText().isEmpty()) {
                    cursor.mergeCharFormat(m_pendingFormat);
                    m_pendingFormat = {};
                }
            });
            initializeChars();
        }
    }

    Q_EMIT textItemChanged();
}

ChatDocumentHandler *ChatDocumentHandler::previousDocumentHandler() const
{
    return m_previousDocumentHandler;
}

void ChatDocumentHandler::setPreviousDocumentHandler(ChatDocumentHandler *previousDocumentHandler)
{
    m_previousDocumentHandler = previousDocumentHandler;
}

ChatDocumentHandler *ChatDocumentHandler::nextDocumentHandler() const
{
    return m_nextDocumentHandler;
}

void ChatDocumentHandler::setNextDocumentHandler(ChatDocumentHandler *nextDocumentHandler)
{
    m_nextDocumentHandler = nextDocumentHandler;
}

QString ChatDocumentHandler::fixedStartChars() const
{
    return m_fixedStartChars;
}

void ChatDocumentHandler::setFixedStartChars(const QString &chars)
{
    if (chars == m_fixedStartChars) {
        return;
    }
    m_fixedStartChars = chars;
}

QString ChatDocumentHandler::fixedEndChars() const
{
    return m_fixedEndChars;
    ;
}

void ChatDocumentHandler::setFixedEndChars(const QString &chars)
{
    if (chars == m_fixedEndChars) {
        return;
    }
    m_fixedEndChars = chars;
}

QString ChatDocumentHandler::initialText() const
{
    return m_initialText;
}

void ChatDocumentHandler::setInitialText(const QString &text)
{
    if (text == m_initialText) {
        return;
    }
    m_initialText = text;
}

void ChatDocumentHandler::initializeChars()
{
    const auto doc = document();
    if (!doc) {
        return;
    }

    QTextCursor cursor = QTextCursor(doc);
    if (cursor.isNull()) {
        return;
    }

    if (doc->isEmpty() && !m_initialText.isEmpty()) {
        cursor.insertText(m_initialText);
    }

    if (!m_fixedStartChars.isEmpty() && doc->characterAt(0) != m_fixedStartChars) {
        cursor.movePosition(QTextCursor::Start);
        cursor.insertText(m_fixedEndChars);
    }

    if (!m_fixedStartChars.isEmpty() && doc->characterAt(doc->characterCount()) != m_fixedStartChars) {
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(m_fixedEndChars);
    }
}

QTextDocument *ChatDocumentHandler::document() const
{
    if (!m_textItem) {
        return nullptr;
    }
    const auto quickDocument = qvariant_cast<QQuickTextDocument *>(m_textItem->property("textDocument"));
    return quickDocument ? quickDocument->textDocument() : nullptr;
}

int ChatDocumentHandler::cursorPosition() const
{
    if (!m_textItem) {
        return -1;
    }
    return m_textItem->property("cursorPosition").toInt();
}

void ChatDocumentHandler::updateCursor()
{
    int start = completionStartIndex();
    m_completionModel->setText(getText().mid(start, cursorPosition() - start), getText().mid(start));

    Q_EMIT atFirstLineChanged();
    Q_EMIT atLastLineChanged();
}

int ChatDocumentHandler::selectionStart() const
{
    if (!m_textItem) {
        return -1;
    }
    return m_textItem->property("selectionStart").toInt();
}

int ChatDocumentHandler::selectionEnd() const
{
    if (!m_textItem) {
        return -1;
    }
    return m_textItem->property("selectionEnd").toInt();
}

QTextCursor ChatDocumentHandler::textCursor() const
{
    if (!document()) {
        return QTextCursor();
    }

    QTextCursor cursor = QTextCursor(document());
    if (selectionStart() != selectionEnd()) {
        cursor.setPosition(selectionStart());
        cursor.setPosition(selectionEnd(), QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(cursorPosition());
    }
    return cursor;
}

bool ChatDocumentHandler::isEmpty() const
{
    return htmlText().length() == 0;
}

bool ChatDocumentHandler::atFirstLine() const
{
    const auto cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return cursor.blockNumber() == 0 && cursor.block().layout()->lineForTextPosition(cursor.positionInBlock()).lineNumber() == 0;
}

bool ChatDocumentHandler::atLastLine() const
{
    const auto cursor = textCursor();
    const auto doc = document();
    if (cursor.isNull() || !doc) {
        return false;
    }
    return cursor.blockNumber() == doc->blockCount() - 1
        && cursor.block().layout()->lineForTextPosition(cursor.positionInBlock()).lineNumber() == (cursor.block().layout()->lineCount() - 1);
}

void ChatDocumentHandler::setCursorFromDocumentHandler(ChatDocumentHandler *previousDocumentHandler, bool infront, int defaultPosition)
{
    const auto doc = document();
    const auto item = textItem();
    if (!doc || !item) {
        return;
    }

    item->forceActiveFocus();

    if (!previousDocumentHandler) {
        const auto docLastBlockLayout = doc->lastBlock().layout();
        item->setProperty("cursorPosition", infront ? defaultPosition : docLastBlockLayout->lineAt(docLastBlockLayout->lineCount() - 1).textStart());
        item->setProperty("cursorVisible", true);
        return;
    }

    const auto previousLinePosition = previousDocumentHandler->cursorPositionInLine();
    const auto newMaxLineLength = lineLength(infront ? 0 : lineCount() - 1);
    item->setProperty("cursorPosition",
                      std::min(previousLinePosition, newMaxLineLength ? *newMaxLineLength : defaultPosition) + (infront ? 0 : doc->lastBlock().position()));
    item->setProperty("cursorVisible", true);
}

int ChatDocumentHandler::lineCount() const
{
    if (const auto doc = document()) {
        return doc->lineCount();
    }
    return 0;
}

std::optional<int> ChatDocumentHandler::lineLength(int lineNumber) const
{
    const auto doc = document();
    if (!doc || lineNumber < 0 || lineNumber >= doc->lineCount()) {
        return std::nullopt;
    }
    const auto block = doc->findBlockByLineNumber(lineNumber);
    const auto lineNumInBlock = lineNumber - block.firstLineNumber();
    return block.layout()->lineAt(lineNumInBlock).textLength();
}

int ChatDocumentHandler::cursorPositionInLine() const
{
    const auto cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return cursor.positionInBlock();
}

QTextDocumentFragment ChatDocumentHandler::takeFirstBlock()
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
    cursor.endEditBlock();
    if (document()->characterCount() - 1 <= (m_fixedStartChars.length() + m_fixedEndChars.length())) {
        Q_EMIT removeMe(this);
    }
    return block;
}

void ChatDocumentHandler::fillFragments(bool &hasBefore, QTextDocumentFragment &midFragment, std::optional<QTextDocumentFragment> &afterFragment)
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
    cursor.endEditBlock();

    midFragment = cursor.selection();
    if (!midFragment.isEmpty()) {
        cursor.removeSelectedText();
    }
    cursor.deletePreviousChar();
    if (afterBlock) {
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        afterFragment = cursor.selection();
        cursor.removeSelectedText();
    }
}

void ChatDocumentHandler::insertFragment(const QTextDocumentFragment fragment, InsertPosition position, bool keepPosition)
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
    cursor.setPosition(currentPosition);
    if (textFormat() && textFormat() == Qt::PlainText) {
        const auto wasEmpty = isEmpty();
        auto text = fragment.toPlainText();
        while (text.startsWith(u"\n"_s)) {
            text.removeFirst();
        }
        while (text.endsWith(u"\n"_s)) {
            text.removeLast();
        }
        cursor.insertText(fragment.toPlainText());
        if (wasEmpty) {
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.deletePreviousChar();
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.deleteChar();
        }
    } else {
        cursor.insertMarkdown(trim(fragment.toMarkdown()));
    }
    if (keepPosition) {
        cursor.setPosition(currentPosition);
    }
    if (textItem()) {
        textItem()->setProperty("cursorPosition", cursor.position());
    }
}

void ChatDocumentHandler::complete(int index)
{
    if (document() == nullptr) {
        qCWarning(ChatDocumentHandling) << "complete called with m_document set to nullptr.";
        return;
    }
    if (m_completionModel->autoCompletionType() == CompletionModel::None) {
        qCWarning(ChatDocumentHandling) << "complete called with m_completionModel->autoCompletionType() == CompletionModel::None.";
        return;
    }

    // Ensure we only search for the beginning of the current completion identifier
    const auto fromIndex = qMax(completionStartIndex(), 0);

    if (m_completionModel->autoCompletionType() == CompletionModel::User) {
        auto name = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::DisplayNameRole).toString();
        auto id = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::SubtitleRole).toString();
        auto text = getText();
        auto at = text.indexOf(QLatin1Char('@'), fromIndex);
        QTextCursor cursor(document());
        cursor.setPosition(at);
        cursor.setPosition(cursorPosition(), QTextCursor::KeepAnchor);
        cursor.insertText(name + u" "_s);
        cursor.setPosition(at);
        cursor.setPosition(cursor.position() + name.size(), QTextCursor::KeepAnchor);
        cursor.setKeepPositionOnInsert(true);
        pushMention({cursor, name, 0, 0, id});
        m_highlighter->rehighlight();
    } else if (m_completionModel->autoCompletionType() == CompletionModel::Command) {
        auto command = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::ReplacedTextRole).toString();
        auto text = getText();
        auto at = text.indexOf(QLatin1Char('/'), fromIndex);
        QTextCursor cursor(document());
        cursor.setPosition(at);
        cursor.setPosition(cursorPosition(), QTextCursor::KeepAnchor);
        cursor.insertText(u"/%1 "_s.arg(command));
    } else if (m_completionModel->autoCompletionType() == CompletionModel::Room) {
        auto alias = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::SubtitleRole).toString();
        auto text = getText();
        auto at = text.indexOf(QLatin1Char('#'), fromIndex);
        QTextCursor cursor(document());
        cursor.setPosition(at);
        cursor.setPosition(cursorPosition(), QTextCursor::KeepAnchor);
        cursor.insertText(alias + u" "_s);
        cursor.setPosition(at);
        cursor.setPosition(cursor.position() + alias.size(), QTextCursor::KeepAnchor);
        cursor.setKeepPositionOnInsert(true);
        pushMention({cursor, alias, 0, 0, alias});
        m_highlighter->rehighlight();
    } else if (m_completionModel->autoCompletionType() == CompletionModel::Emoji) {
        auto shortcode = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::ReplacedTextRole).toString();
        auto text = getText();
        auto at = text.indexOf(QLatin1Char(':'), fromIndex);
        QTextCursor cursor(document());
        cursor.setPosition(at);
        cursor.setPosition(cursorPosition(), QTextCursor::KeepAnchor);
        cursor.insertText(shortcode);
    }
}

CompletionModel *ChatDocumentHandler::completionModel() const
{
    return m_completionModel;
}

QString ChatDocumentHandler::getText() const
{
    if (!document()) {
        qCWarning(ChatDocumentHandling) << "getText called with no QQuickTextDocument available.";
        return {};
    }
    return document()->toPlainText();
}

void ChatDocumentHandler::pushMention(const Mention mention) const
{
    if (!m_room || m_type == ChatBarType::None) {
        qCWarning(ChatDocumentHandling) << "pushMention called with no ChatBarCache available. ChatBarType: " << m_type << " Room: " << m_room;
        return;
    }
    m_room->cacheForType(m_type)->mentions()->push_back(mention);
}

void ChatDocumentHandler::updateMentions(const QString &editId)
{
    if (editId.isEmpty() || m_type == ChatBarType::None || !m_room) {
        return;
    }

    if (auto event = m_room->findInTimeline(editId); event != m_room->historyEdge()) {
        if (const auto &roomMessageEvent = &*event->viewAs<Quotient::RoomMessageEvent>()) {
            // Replaces the mentions that are baked into the HTML but plaintext in the original markdown
            const QRegularExpression re(uR"lit(<a\shref="https:\/\/matrix.to\/#\/([\S]*)"\s?>([\S]*)<\/a>)lit"_s);

            m_room->cacheForType(m_type)->mentions()->clear();

            int linkSize = 0;
            auto matches = re.globalMatch(EventHandler::rawMessageBody(*roomMessageEvent));
            while (matches.hasNext()) {
                const QRegularExpressionMatch match = matches.next();
                if (match.hasMatch()) {
                    const QString id = match.captured(1);
                    const QString name = match.captured(2);

                    const int position = match.capturedStart(0) - linkSize;
                    const int end = position + name.length();
                    linkSize += match.capturedLength(0) - name.length();

                    QTextCursor cursor(document());
                    cursor.setPosition(position);
                    cursor.setPosition(end, QTextCursor::KeepAnchor);
                    cursor.setKeepPositionOnInsert(true);

                    pushMention(Mention{.cursor = cursor, .text = name, .start = position, .position = end, .id = id});
                }
            }
        }
    }
}

void ChatDocumentHandler::setTextColor(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    mergeFormatOnWordOrSelection(format);
    Q_EMIT textColorChanged();
}

bool ChatDocumentHandler::bold() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return textCursor().charFormat().fontWeight() == QFont::Bold;
}

bool ChatDocumentHandler::italic() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontItalic();
}

bool ChatDocumentHandler::underline() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontUnderline();
}

bool ChatDocumentHandler::strikethrough() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontStrikeOut();
}

QColor ChatDocumentHandler::textColor() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QColor(Qt::black);
    QTextCharFormat format = cursor.charFormat();
    return format.foreground().color();
}

std::optional<Qt::TextFormat> ChatDocumentHandler::textFormat() const
{
    if (!m_textItem) {
        return std::nullopt;
    }

    return static_cast<Qt::TextFormat>(m_textItem->property("textFormat").toInt());
}

void ChatDocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    if (cursor.hasSelection()) {
        cursor.mergeCharFormat(format);
    } else {
        m_pendingFormat = format.toCharFormat();
    }
}

QString ChatDocumentHandler::currentLinkText() const
{
    QTextCursor cursor = textCursor();
    selectLinkText(&cursor);
    return cursor.selectedText();
}

void ChatDocumentHandler::selectLinkText(QTextCursor *cursor) const
{
    // If the cursor is on a link, select the text of the link.
    if (cursor->charFormat().isAnchor()) {
        const QString aHref = cursor->charFormat().anchorHref();

        // Move cursor to start of link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atStart()) {
                break;
            }
            cursor->setPosition(cursor->position() - 1);
        }
        if (cursor->charFormat().anchorHref() != aHref) {
            cursor->setPosition(cursor->position() + 1, QTextCursor::KeepAnchor);
        }

        // Move selection to the end of the link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atEnd()) {
                break;
            }
            const int oldPosition = cursor->position();
            cursor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            // Wordaround Qt Bug. when we have a table.
            // FIXME selection url
            if (oldPosition == cursor->position()) {
                break;
            }
        }
        if (cursor->charFormat().anchorHref() != aHref) {
            cursor->setPosition(cursor->position() - 1, QTextCursor::KeepAnchor);
        }
    } else if (cursor->hasSelection()) {
        // Nothing to do. Using the currently selected text as the link text.
    } else {
        // Select current word
        cursor->movePosition(QTextCursor::StartOfWord);
        cursor->movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }
}

void ChatDocumentHandler::insertImage(const QUrl &url)
{
    if (!url.isLocalFile()) {
        return;
    }

    QImage image;
    if (!image.load(url.path())) {
        return;
    }

    // Ensure we are putting the image in a new line and not in a list has it
    // breaks the Qt rendering
    textCursor().insertHtml(QStringLiteral("<br />"));

    while (canDedentList()) {
        m_nestedListHelper.handleOnIndentLess(textCursor());
    }

    textCursor().insertHtml(QStringLiteral("<img width=\"500\" src=\"") + url.path() + QStringLiteral("\"\\>"));
}

void ChatDocumentHandler::insertTable(int rows, int columns)
{
    QString htmlText;

    QTextCursor cursor = textCursor();
    QTextTableFormat tableFormat;
    tableFormat.setBorder(1);
    const int numberOfColumns(columns);
    QList<QTextLength> constrains;
    constrains.reserve(numberOfColumns);
    const QTextLength::Type type = QTextLength::PercentageLength;
    const int length = 100; // 100% of window width

    const QTextLength textlength(type, length / numberOfColumns);
    for (int i = 0; i < numberOfColumns; ++i) {
        constrains.append(textlength);
    }
    tableFormat.setColumnWidthConstraints(constrains);
    tableFormat.setAlignment(Qt::AlignLeft);
    tableFormat.setCellSpacing(0);
    tableFormat.setCellPadding(4);
    tableFormat.setBorderCollapse(true);
    tableFormat.setBorder(0.5);
    tableFormat.setTopMargin(20);

    Q_ASSERT(cursor.document());
    QTextTable *table = cursor.insertTable(rows, numberOfColumns, tableFormat);

    // fill table with whitespace
    for (int i = 0, rows = table->rows(); i < rows; i++) {
        for (int j = 0, columns = table->columns(); j < columns; j++) {
            auto cell = table->cellAt(i, j);
            Q_ASSERT(cell.isValid());
            cell.firstCursorPosition().insertText(QStringLiteral(" "));
        }
    }
    return;
}

void ChatDocumentHandler::updateLink(const QString &linkUrl, const QString &linkText)
{
    auto cursor = textCursor();
    selectLinkText(&cursor);

    cursor.beginEditBlock();

    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format = cursor.charFormat();
    // Save original format to create an extra space with the existing char
    // format for the block
    if (!linkUrl.isEmpty()) {
        // Add link details
        format.setAnchor(true);
        format.setAnchorHref(linkUrl);
        // Workaround for QTBUG-1814:
        // Link formatting does not get applied immediately when setAnchor(true)
        // is called.  So the formatting needs to be applied manually.
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        format.setUnderlineColor(linkColor());
        format.setForeground(linkColor());
    } else {
        // Remove link details
        format.setAnchor(false);
        format.setAnchorHref(QString());
        // Workaround for QTBUG-1814:
        // Link formatting does not get removed immediately when setAnchor(false)
        // is called. So the formatting needs to be applied manually.
        QTextDocument defaultTextDocument;
        QTextCharFormat defaultCharFormat = defaultTextDocument.begin().charFormat();

        format.setUnderlineStyle(defaultCharFormat.underlineStyle());
        format.setUnderlineColor(defaultCharFormat.underlineColor());
        format.setForeground(defaultCharFormat.foreground());
    }

    // Insert link text specified in dialog, otherwise write out url.
    QString _linkText;
    if (!linkText.isEmpty()) {
        _linkText = linkText;
    } else {
        _linkText = linkUrl;
    }
    cursor.insertText(_linkText, format);

    cursor.endEditBlock();
}

QColor ChatDocumentHandler::linkColor()
{
    if (mLinkColor.isValid()) {
        return mLinkColor;
    }
    regenerateColorScheme();
    return mLinkColor;
}

void ChatDocumentHandler::regenerateColorScheme()
{
    mLinkColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color();
    // TODO update existing link
}

void ChatDocumentHandler::setFormat(RichFormat::Format format)
{
    switch (RichFormat::typeForFormat(format)) {
    case RichFormat::Text:
        setTextFormat(format);
        return;
    case RichFormat::List:
        setListFormat(format);
        return;
    case RichFormat::Style:
        setStyleFormat(format);
        return;
    default:
        return;
    }
}

void ChatDocumentHandler::indentListMore()
{
    m_nestedListHelper.handleOnIndentMore(textCursor());
    Q_EMIT currentListStyleChanged();
}

void ChatDocumentHandler::indentListLess()
{
    m_nestedListHelper.handleOnIndentLess(textCursor());
    Q_EMIT currentListStyleChanged();
}

void ChatDocumentHandler::setListFormat(RichFormat::Format format)
{
    m_nestedListHelper.handleOnBulletType(RichFormat::listStyleForFormat(format), textCursor());
    Q_EMIT currentListStyleChanged();
}

bool ChatDocumentHandler::canIndentList() const
{
    return m_nestedListHelper.canIndent(textCursor()) && textCursor().blockFormat().headingLevel() == 0;
}

bool ChatDocumentHandler::canDedentList() const
{
    return m_nestedListHelper.canDedent(textCursor()) && textCursor().blockFormat().headingLevel() == 0;
}

int ChatDocumentHandler::currentListStyle() const
{
    if (!textCursor().currentList()) {
        return 0;
    }

    return -textCursor().currentList()->format().style();
}

void ChatDocumentHandler::setTextFormat(RichFormat::Format format)
{
    if (RichFormat::typeForFormat(format) != RichFormat::Text) {
        return;
    }
    mergeFormatOnWordOrSelection(RichFormat::charFormatForFormat(format, RichFormat::hasFormat(textCursor(), format)));
    Q_EMIT formatChanged();
}

RichFormat::Format ChatDocumentHandler::style() const
{
    return static_cast<RichFormat::Format>(textCursor().blockFormat().headingLevel());
}

void ChatDocumentHandler::setStyleFormat(RichFormat::Format format)
{
    // Paragraph is special because it is normally a Block format but if we're already
    // in a Paragraph it clears any existing style.
    if (!(RichFormat::typeForFormat(format) == RichFormat::Style || format == RichFormat::Paragraph)) {
        return;
    }

    qWarning() << format;

    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
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

    Q_EMIT formatChanged();
    Q_EMIT styleChanged();
}

void ChatDocumentHandler::tab()
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (cursor.currentList()) {
        indentListMore();
        return;
    }
    insertText(u"	"_s);
}

void ChatDocumentHandler::deleteChar()
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (cursor.position() >= document()->characterCount() - m_fixedEndChars.length() - 1) {
        if (const auto nextHandler = nextDocumentHandler()) {
            insertFragment(nextHandler->takeFirstBlock(), Cursor, true);
        }
        return;
    }
    cursor.deleteChar();
}

void ChatDocumentHandler::backspace()
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }
    if (cursor.position() <= m_fixedStartChars.length()) {
        qWarning() << "unhandled backspace";
        if (cursor.currentList()) {
            indentListLess();
            return;
        }
        if (const auto previousHandler = previousDocumentHandler()) {
            previousHandler->insertFragment(takeFirstBlock(), End, true);
        } else {
            Q_EMIT unhandledBackspaceAtBeginning(this);
        }
        return;
    }
    cursor.deletePreviousChar();
}

void ChatDocumentHandler::insertReturn()
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return;
    }
    cursor.insertBlock();
}

void ChatDocumentHandler::insertText(const QString &text)
{
    textCursor().insertText(text);
}

QString ChatDocumentHandler::currentLinkUrl() const
{
    return textCursor().charFormat().anchorHref();
}

void ChatDocumentHandler::dumpHtml()
{
    qWarning() << htmlText();
}

QString ChatDocumentHandler::htmlText() const
{
    const auto doc = document();
    if (!doc) {
        return {};
    }
    return trim(doc->toMarkdown());
}

QString ChatDocumentHandler::trim(QString string) const
{
    while (string.startsWith(u"\n"_s)) {
        string.removeFirst();
    }
    while (string.endsWith(u"\n"_s)) {
        string.removeLast();
    }
    return string;
}

#include "moc_chatdocumenthandler.cpp"
