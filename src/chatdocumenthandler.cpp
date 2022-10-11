// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chatdocumenthandler.h"

#include <QQmlFile>
#include <QQmlFileSelector>
#include <QQuickTextDocument>
#include <QStringBuilder>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextDocument>

#include <Sonnet/BackgroundChecker>
#include <Sonnet/Settings>

#include "actionsmodel.h"
#include "completionmodel.h"
#include "neochatroom.h"
#include "roomlistmodel.h"

class SyntaxHighlighter : public QSyntaxHighlighter
{
public:
    QTextCharFormat mentionFormat;
    QTextCharFormat errorFormat;
    Sonnet::BackgroundChecker *checker = new Sonnet::BackgroundChecker;
    Sonnet::Settings settings;
    QList<QPair<int, QString>> errors;
    QString previousText;
    SyntaxHighlighter(QObject *parent)
        : QSyntaxHighlighter(parent)
    {
        mentionFormat.setFontWeight(QFont::Bold);
        mentionFormat.setForeground(Qt::blue);

        errorFormat.setForeground(Qt::red);
        errorFormat.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);

        connect(checker, &Sonnet::BackgroundChecker::misspelling, this, [this](const QString &word, int start) {
            errors += {start, word};
            rehighlight();
            checker->continueChecking();
        });
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
        auto room = dynamic_cast<ChatDocumentHandler *>(parent())->room();
        if (!room) {
            return;
        }
        auto mentions = room->mentions();
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
};

ChatDocumentHandler::ChatDocumentHandler(QObject *parent)
    : QObject(parent)
    , m_document(nullptr)
    , m_cursorPosition(-1)
    , m_highlighter(new SyntaxHighlighter(this))
    , m_completionModel(new CompletionModel())
{
    connect(this, &ChatDocumentHandler::roomChanged, this, [this]() {
        m_completionModel->setRoom(m_room);
        static NeoChatRoom *previousRoom = nullptr;
        if (previousRoom) {
            disconnect(nullptr, &NeoChatRoom::chatBoxTextChanged, this, nullptr);
        }
        previousRoom = m_room;
        connect(m_room, &NeoChatRoom::chatBoxTextChanged, this, [this]() {
            int start = completionStartIndex();
            m_completionModel->setText(m_room->chatBoxText().mid(start, cursorPosition() - start), m_room->chatBoxText().mid(start));
        });
    });
    connect(this, &ChatDocumentHandler::documentChanged, this, [this]() {
        m_highlighter->setDocument(m_document->textDocument());
    });
    connect(this, &ChatDocumentHandler::cursorPositionChanged, this, [this]() {
        if (!m_room) {
            return;
        }
        int start = completionStartIndex();
        m_completionModel->setText(m_room->chatBoxText().mid(start, cursorPosition() - start), m_room->chatBoxText().mid(start));
    });
}

int ChatDocumentHandler::completionStartIndex() const
{
    if (!m_room) {
        return 0;
    }

    const auto &cursor = cursorPosition();
    const auto &text = m_room->chatBoxText();
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

QQuickTextDocument *ChatDocumentHandler::document() const
{
    return m_document;
}

void ChatDocumentHandler::setDocument(QQuickTextDocument *document)
{
    if (document == m_document) {
        return;
    }

    if (m_document) {
        m_document->textDocument()->disconnect(this);
    }
    m_document = document;
    Q_EMIT documentChanged();
}

int ChatDocumentHandler::cursorPosition() const
{
    return m_cursorPosition;
}

void ChatDocumentHandler::setCursorPosition(int position)
{
    if (position == m_cursorPosition) {
        return;
    }
    if (m_room) {
        m_cursorPosition = position;
    }
    Q_EMIT cursorPositionChanged();
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
    Q_EMIT roomChanged();
}

void ChatDocumentHandler::complete(int index)
{
    if (m_completionModel->autoCompletionType() == ChatDocumentHandler::User) {
        auto name = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::Text).toString();
        auto id = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::Subtitle).toString();
        auto text = m_room->chatBoxText();
        auto at = text.lastIndexOf(QLatin1Char('@'), cursorPosition() - 1);
        QTextCursor cursor(document()->textDocument());
        cursor.setPosition(at);
        cursor.setPosition(cursorPosition(), QTextCursor::KeepAnchor);
        cursor.insertText(name % " ");
        cursor.setPosition(at);
        cursor.setPosition(cursor.position() + name.size(), QTextCursor::KeepAnchor);
        cursor.setKeepPositionOnInsert(true);
        m_room->mentions()->push_back({cursor, name, 0, 0, id});
        m_highlighter->rehighlight();
    } else if (m_completionModel->autoCompletionType() == ChatDocumentHandler::Command) {
        auto command = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::ReplacedText).toString();
        auto text = m_room->chatBoxText();
        auto at = text.lastIndexOf(QLatin1Char('/'));
        QTextCursor cursor(document()->textDocument());
        cursor.setPosition(at);
        cursor.setPosition(cursorPosition(), QTextCursor::KeepAnchor);
        cursor.insertText(QStringLiteral("/%1 ").arg(command));
    } else if (m_completionModel->autoCompletionType() == ChatDocumentHandler::Room) {
        auto alias = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::Subtitle).toString();
        auto text = m_room->chatBoxText();
        auto at = text.lastIndexOf(QLatin1Char('#'), cursorPosition() - 1);
        QTextCursor cursor(document()->textDocument());
        cursor.setPosition(at);
        cursor.setPosition(cursorPosition(), QTextCursor::KeepAnchor);
        cursor.insertText(alias % " ");
        cursor.setPosition(at);
        cursor.setPosition(cursor.position() + alias.size(), QTextCursor::KeepAnchor);
        cursor.setKeepPositionOnInsert(true);
        m_room->mentions()->push_back({cursor, alias, 0, 0, alias});
        m_highlighter->rehighlight();
    } else if (m_completionModel->autoCompletionType() == ChatDocumentHandler::Emoji) {
        auto shortcode = m_completionModel->data(m_completionModel->index(index, 0), CompletionModel::Text).toString();
        auto text = m_room->chatBoxText();
        auto at = text.lastIndexOf(QLatin1Char(':'));
        QTextCursor cursor(document()->textDocument());
        cursor.setPosition(at);
        cursor.setPosition(cursorPosition(), QTextCursor::KeepAnchor);
        cursor.insertText(shortcode);
    }
}

CompletionModel *ChatDocumentHandler::completionModel() const
{
    return m_completionModel;
}

int ChatDocumentHandler::selectionStart() const
{
    return m_selectionStart;
}

void ChatDocumentHandler::setSelectionStart(int position)
{
    if (position == m_selectionStart) {
        return;
    }

    m_selectionStart = position;
    Q_EMIT selectionStartChanged();
}

int ChatDocumentHandler::selectionEnd() const
{
    return m_selectionEnd;
}

void ChatDocumentHandler::setSelectionEnd(int position)
{
    if (position == m_selectionEnd) {
        return;
    }

    m_selectionEnd = position;
    Q_EMIT selectionEndChanged();
}
