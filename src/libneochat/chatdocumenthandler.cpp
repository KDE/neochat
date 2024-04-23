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
#include <QTextList>
#include <QTextTable>
#include <QTimer>

#include <Kirigami/Platform/PlatformTheme>
#include <KColorScheme>

#include <Sonnet/BackgroundChecker>
#include <Sonnet/Settings>

#include "chatbartype.h"
#include "chatdocumenthandler_logging.h"
#include "eventhandler.h"
#include "utils.h"

using namespace Qt::StringLiterals;

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
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
        const auto chatchache = handler->chatBarCache();
        if (!chatchache) {
            return;
        }
        auto mentions = chatchache->mentions();
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
}

void ChatDocumentHandler::updateCompletion() const
{
    int start = completionStartIndex();
    m_completionModel->setText(getText().mid(start, cursorPosition() - start), getText().mid(start));
}

int ChatDocumentHandler::completionStartIndex() const
{
    if (!m_room) {
        return 0;
    }

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
        connect(m_textItem, SIGNAL(cursorPositionChanged()), this, SLOT(updateCompletion()));
        if (document()) {
            connect(document(), &QTextDocument::contentsChanged, this, [this]() {
                if (m_room) {
                    m_room->cacheForType(m_type)->setText(getText());
                    int start = completionStartIndex();
                    m_completionModel->setText(getText().mid(start, cursorPosition() - start), getText().mid(start));
                }
            });
        }
    }

    Q_EMIT textItemChanged();
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

NeoChatRoom *ChatDocumentHandler::room() const
{
    return m_room;
}

void ChatDocumentHandler::setRoom(NeoChatRoom *room)
{
    if (m_room == room) {
        return;
    }

    if (m_room && m_type != ChatBarType::None) {
        m_room->cacheForType(m_type)->disconnect(this);
        if (!m_room->isSpace() && document() && m_type == ChatBarType::Room) {
            m_room->mainCache()->setSavedText(document()->toPlainText());
        }
    }

    m_room = room;

    m_completionModel->setRoom(m_room);
    if (m_room && m_type != ChatBarType::None) {
        connect(m_room->cacheForType(m_type), &ChatBarCache::textChanged, this, [this]() {
            int start = completionStartIndex();
            m_completionModel->setText(getText().mid(start, cursorPosition() - start), getText().mid(start));
            Q_EMIT fontFamilyChanged();
            Q_EMIT textColorChanged();
            Q_EMIT alignmentChanged();
            Q_EMIT boldChanged();
            Q_EMIT italicChanged();
            Q_EMIT underlineChanged();
            Q_EMIT checkableChanged();
            Q_EMIT strikethroughChanged();
            Q_EMIT fontSizeChanged();
            Q_EMIT fileUrlChanged();
        });
        if (!m_room->isSpace() && document() && m_type == ChatBarType::Room) {
            document()->setPlainText(room->mainCache()->savedText());
            m_room->mainCache()->setText(room->mainCache()->savedText());
        }
    }

    Q_EMIT roomChanged();
}

ChatBarCache *ChatDocumentHandler::chatBarCache() const
{
    if (!m_room || m_type == ChatBarType::None) {
        return nullptr;
    }
    return m_room->cacheForType(m_type);
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

void ChatDocumentHandler::setFontSize(int size)
{
    if (size <= 0)
        return;

    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return;

    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    if (cursor.charFormat().property(QTextFormat::FontPointSize).toInt() == size)
        return;

    QTextCharFormat format;
    format.setFontPointSize(size);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT fontSizeChanged();
}

void ChatDocumentHandler::setStrikethrough(bool strikethrough)
{
    QTextCharFormat format;
    format.setFontStrikeOut(strikethrough);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT underlineChanged();
}

void ChatDocumentHandler::setTextColor(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    mergeFormatOnWordOrSelection(format);
    Q_EMIT textColorChanged();
}

Qt::Alignment ChatDocumentHandler::alignment() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return Qt::AlignLeft;
    return textCursor().blockFormat().alignment();
}

void ChatDocumentHandler::setAlignment(Qt::Alignment alignment)
{
    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    Q_EMIT alignmentChanged();
}

bool ChatDocumentHandler::bold() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull()) {
        return false;
    }
    return textCursor().charFormat().fontWeight() == QFont::Bold;
}

void ChatDocumentHandler::setBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT boldChanged();
}

bool ChatDocumentHandler::italic() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontItalic();
}

void ChatDocumentHandler::setItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT italicChanged();
}

bool ChatDocumentHandler::underline() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontUnderline();
}

void ChatDocumentHandler::setUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT underlineChanged();
}

bool ChatDocumentHandler::strikethrough() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontStrikeOut();
}

QString ChatDocumentHandler::fontFamily() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QString();
    QTextCharFormat format = cursor.charFormat();
    return format.font().family();
}

void ChatDocumentHandler::setFontFamily(const QString &family)
{
    QTextCharFormat format;
    format.setFontFamilies({family});
    mergeFormatOnWordOrSelection(format);
    Q_EMIT fontFamilyChanged();
}

QColor ChatDocumentHandler::textColor() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QColor(Qt::black);
    QTextCharFormat format = cursor.charFormat();
    return format.foreground().color();
}

QTextCursor ChatDocumentHandler::textCursor() const
{
    QTextDocument *doc = document();
    if (!doc)
        return QTextCursor();

    QTextCursor cursor = QTextCursor(doc);
    if (selectionStart() != selectionEnd()) {
        cursor.setPosition(selectionStart());
        cursor.setPosition(selectionEnd(), QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(cursorPosition());
    }
    return cursor;
}

void ChatDocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
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

int ChatDocumentHandler::currentHeadingLevel() const
{
    return textCursor().blockFormat().headingLevel();
}

void ChatDocumentHandler::indentListMore()
{
    m_nestedListHelper.handleOnIndentMore(textCursor());
}

void ChatDocumentHandler::indentListLess()
{
    m_nestedListHelper.handleOnIndentLess(textCursor());
}

void ChatDocumentHandler::setListStyle(int styleIndex)
{
    m_nestedListHelper.handleOnBulletType(-styleIndex, textCursor());
    Q_EMIT currentListStyleChanged();
}

void ChatDocumentHandler::setHeadingLevel(int level)
{
    const int boundedLevel = qBound(0, 6, level);
    // Apparently, 5 is maximum for FontSizeAdjustment; otherwise level=1 and
    // level=2 look the same
    const int sizeAdjustment = boundedLevel > 0 ? 5 - boundedLevel : 0;

    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();

    QTextBlockFormat blkfmt;
    blkfmt.setHeadingLevel(boundedLevel);
    cursor.mergeBlockFormat(blkfmt);

    QTextCharFormat chrfmt;
    chrfmt.setFontWeight(boundedLevel > 0 ? QFont::Bold : QFont::Normal);
    chrfmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
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
    selectCursor.mergeCharFormat(chrfmt);

    cursor.mergeBlockCharFormat(chrfmt);
    cursor.endEditBlock();
    // richTextComposer()->setTextCursor(cursor);
    // richTextComposer()->setFocus();
    // richTextComposer()->activateRichText();
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

int ChatDocumentHandler::fontSize() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return 0;
    QTextCharFormat format = cursor.charFormat();
    return format.font().pointSize();
}

QString ChatDocumentHandler::fileName() const
{
    const QString filePath = QQmlFile::urlToLocalFileOrQrc(m_fileUrl);
    const QString fileName = QFileInfo(filePath).fileName();
    if (fileName.isEmpty())
        return QStringLiteral("untitled.txt");
    return fileName;
}

QString ChatDocumentHandler::fileType() const
{
    return QFileInfo(fileName()).suffix();
}

QUrl ChatDocumentHandler::fileUrl() const
{
    return m_fileUrl;
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

QString ChatDocumentHandler::htmlText()
{
    auto text = document()->toMarkdown();
    while (text.startsWith(u"\n"_s)) {
        text.remove(0, 1);
    }
    while (text.endsWith(u"\n"_s)) {
        text.remove(text.size() - 1, text.size());
    }
    return text;
}

#include "moc_chatdocumenthandler.cpp"
