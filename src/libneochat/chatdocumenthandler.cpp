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
#include <QTimer>

#include <Kirigami/Platform/PlatformTheme>
#include <Sonnet/BackgroundChecker>
#include <Sonnet/Settings>

#include "chatbartype.h"
#include "chatdocumenthandler_logging.h"
#include "eventhandler.h"

using namespace Qt::StringLiterals;

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
    QTextCharFormat mentionFormat;
    QTextCharFormat errorFormat;
    Sonnet::BackgroundChecker *checker = new Sonnet::BackgroundChecker;
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

        connect(checker, &Sonnet::BackgroundChecker::misspelling, this, [this](const QString &word, int start) {
            errors += {start, word};
            checker->continueChecking();
        });
        connect(checker, &Sonnet::BackgroundChecker::done, this, [this]() {
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
                checker->stop();
                errors.clear();
                checker->setText(text);
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
    return document()->toRawText();
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

#include "moc_chatdocumenthandler.cpp"
