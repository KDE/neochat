// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionshandler.h"

#include <Quotient/csapi/joining.h>
#include <Quotient/events/roommemberevent.h>

#include <cmark.h>

#include <KLocalizedString>
#include <QStringBuilder>

#include "models/actionsmodel.h"
#include "models/customemojimodel.h"
#include "neochatconfig.h"
#include "texthandler.h"

using namespace Quotient;

ActionsHandler::ActionsHandler(QObject *parent)
    : QObject(parent)
{
}

NeoChatRoom *ActionsHandler::room() const
{
    return m_room;
}

void ActionsHandler::setRoom(NeoChatRoom *room)
{
    if (m_room == room) {
        return;
    }

    m_room = room;
    Q_EMIT roomChanged();
}

void ActionsHandler::handleNewMessage()
{
    checkEffects(m_room->chatBoxText());
    if (!m_room->chatBoxAttachmentPath().isEmpty()) {
        QUrl url(m_room->chatBoxAttachmentPath());
        auto path = url.isLocalFile() ? url.toLocalFile() : url.toString();
        m_room->uploadFile(path, m_room->chatBoxText().isEmpty() ? path.mid(path.lastIndexOf('/') + 1) : m_room->chatBoxText());
        m_room->setChatBoxAttachmentPath({});
        m_room->setChatBoxText({});
        return;
    }

    QString handledText = m_room->chatBoxText();
    handledText = handleMentions(handledText);
    handleMessage(m_room->chatBoxText(), handledText);
}

void ActionsHandler::handleEdit()
{
    checkEffects(m_room->editText());

    QString handledText = m_room->editText();
    handledText = handleMentions(handledText, true);
    handleMessage(m_room->editText(), handledText, true);
}

QString ActionsHandler::handleMentions(QString handledText, const bool &isEdit)
{
    if (!m_room) {
        return QString();
    }

    QVector<Mention> *mentions;
    if (isEdit) {
        mentions = m_room->editMentions();
    } else {
        mentions = m_room->mentions();
    }

    std::sort(mentions->begin(), mentions->end(), [](const auto &a, const auto &b) -> bool {
        return a.cursor.anchor() > b.cursor.anchor();
    });

    for (const auto &mention : *mentions) {
        if (mention.text.isEmpty() || mention.id.isEmpty()) {
            continue;
        }
        handledText = handledText.replace(mention.cursor.anchor(),
                                          mention.cursor.position() - mention.cursor.anchor(),
                                          QStringLiteral("[%1](https://matrix.to/#/%2)").arg(mention.text, mention.id));
    }
    mentions->clear();

    return handledText;
}

void ActionsHandler::handleMessage(const QString &text, QString handledText, const bool &isEdit)
{
    if (NeoChatConfig::allowQuickEdit()) {
        QRegularExpression sed("^s/([^/]*)/([^/]*)(/g)?$");
        auto match = sed.match(text);
        if (match.hasMatch()) {
            const QString regex = match.captured(1);
            const QString replacement = match.captured(2).toHtmlEscaped();
            const QString flags = match.captured(3);

            for (auto it = m_room->messageEvents().crbegin(); it != m_room->messageEvents().crend(); it++) {
                if (const auto event = eventCast<const RoomMessageEvent>(&**it)) {
                    if (event->senderId() == m_room->localUser()->id() && event->hasTextContent()) {
                        QString originalString;
                        if (event->content()) {
                            originalString = static_cast<const Quotient::EventContent::TextContent *>(event->content())->body;
                        } else {
                            originalString = event->plainBody();
                        }
                        if (flags == "/g") {
                            m_room->postHtmlMessage(handledText, originalString.replace(regex, replacement), event->msgtype(), "", event->id());
                        } else {
                            m_room->postHtmlMessage(handledText,
                                                    originalString.replace(originalString.indexOf(regex), regex.size(), replacement),
                                                    event->msgtype(),
                                                    "",
                                                    event->id());
                        }
                        return;
                    }
                }
            }
        }
    }
    auto messageType = RoomMessageEvent::MsgType::Text;

    if (handledText.startsWith(QLatin1Char('/'))) {
        for (const auto &action : ActionsModel::instance().allActions()) {
            if (handledText.indexOf(action.prefix) == 1
                && (handledText.indexOf(" ") == action.prefix.length() + 1 || handledText.length() == action.prefix.length() + 1)) {
                handledText = action.handle(handledText.mid(action.prefix.length() + 1).trimmed(), m_room);
                if (action.messageType.has_value()) {
                    messageType = *action.messageType;
                }
                if (action.messageAction) {
                    break;
                } else {
                    return;
                }
            }
        }
    }

    handledText = CustomEmojiModel::instance().preprocessText(handledText);
    TextHandler textHandler;
    textHandler.setData(handledText);
    handledText = textHandler.handleSendText();

    if (handledText.count("<p>") == 1 && handledText.count("</p>") == 1) {
        handledText.remove("<p>");
        handledText.remove("</p>");
    }

    if (handledText.length() == 0) {
        return;
    }

    m_room->postMessage(text, handledText, messageType, m_room->chatBoxReplyId(), isEdit ? m_room->chatBoxEditId() : "");
}

void ActionsHandler::checkEffects(const QString &text)
{
    std::optional<QString> effect = std::nullopt;
    if (text.contains("\u2744")) {
        effect = QLatin1String("snowflake");
    } else if (text.contains("\u1F386")) {
        effect = QLatin1String("fireworks");
    } else if (text.contains("\u2F387")) {
        effect = QLatin1String("fireworks");
    } else if (text.contains("\u1F389")) {
        effect = QLatin1String("confetti");
    } else if (text.contains("\u1F38A")) {
        effect = QLatin1String("confetti");
    }
    if (effect.has_value()) {
        Q_EMIT showEffect(*effect);
    }
}

#include "moc_actionshandler.cpp"
