// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionshandler.h"

#include <Quotient/csapi/joining.h>
#include <Quotient/events/roommemberevent.h>

#include <cmark.h>

#include <KLocalizedString>
#include <QStringBuilder>

#include "models/actionsmodel.h"
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

void ActionsHandler::handleMessageEvent(ChatBarCache *chatBarCache)
{
    if (!m_room || !chatBarCache) {
        qWarning() << "ActionsHandler::handleMessageEvent - called with m_room and/or chatBarCache set to nullptr.";
        return;
    }

    checkEffects(chatBarCache->text());
    if (!chatBarCache->attachmentPath().isEmpty()) {
        QUrl url(chatBarCache->attachmentPath());
        auto path = url.isLocalFile() ? url.toLocalFile() : url.toString();
        m_room->uploadFile(QUrl(path), chatBarCache->text().isEmpty() ? path.mid(path.lastIndexOf(u'/') + 1) : chatBarCache->text());
        chatBarCache->setAttachmentPath({});
        chatBarCache->setText({});
        return;
    }

    QString handledText = chatBarCache->text();
    handledText = handleMentions(handledText, chatBarCache->mentions());
    handleMessage(m_room->mainCache()->text(), handledText, chatBarCache);
}

QString ActionsHandler::handleMentions(QString handledText, QList<Mention> *mentions)
{
    std::sort(mentions->begin(), mentions->end(), [](const auto &a, const auto &b) -> bool {
        return a.cursor.anchor() > b.cursor.anchor();
    });

    for (const auto &mention : *mentions) {
        if (mention.text.isEmpty() || mention.id.isEmpty()) {
            continue;
        }
        handledText = handledText.replace(mention.cursor.anchor(),
                                          mention.cursor.position() - mention.cursor.anchor(),
                                          QStringLiteral("[%1](https://matrix.to/#/%2)").arg(mention.text.toHtmlEscaped(), mention.id));
    }
    mentions->clear();

    return handledText;
}

void ActionsHandler::handleMessage(const QString &text, QString handledText, ChatBarCache *chatBarCache)
{
    Q_ASSERT(m_room);
    if (NeoChatConfig::allowQuickEdit()) {
        QRegularExpression sed(QStringLiteral("^s/([^/]*)/([^/]*)(/g)?$"));
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
                        if (flags == "/g"_ls) {
                            m_room->postHtmlMessage(handledText, originalString.replace(regex, replacement), event->msgtype(), {}, event->id());
                        } else {
                            m_room->postHtmlMessage(handledText,
                                                    originalString.replace(originalString.indexOf(regex), regex.size(), replacement),
                                                    event->msgtype(),
                                                    {},
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
                && (handledText.indexOf(" "_ls) == action.prefix.length() + 1 || handledText.length() == action.prefix.length() + 1)) {
                handledText = action.handle(handledText.mid(action.prefix.length() + 1).trimmed(), m_room, chatBarCache);
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

    TextHandler textHandler;
    textHandler.setData(handledText);
    handledText = textHandler.handleSendText();

    if (handledText.count("<p>"_ls) == 1 && handledText.count("</p>"_ls) == 1) {
        handledText.remove("<p>"_ls);
        handledText.remove("</p>"_ls);
    }

    if (handledText.length() == 0) {
        return;
    }

    m_room->postMessage(text, handledText, messageType, chatBarCache->replyId(), chatBarCache->editId(), chatBarCache->threadId());
}

void ActionsHandler::checkEffects(const QString &text)
{
    std::optional<QString> effect = std::nullopt;
    if (text.contains(QStringLiteral("\u2744"))) {
        effect = QLatin1String("snowflake");
    } else if (text.contains(QStringLiteral("\u1F386"))) {
        effect = QLatin1String("fireworks");
    } else if (text.contains(QStringLiteral("\u2F387"))) {
        effect = QLatin1String("fireworks");
    } else if (text.contains(QStringLiteral("\u1F389"))) {
        effect = QLatin1String("confetti");
    } else if (text.contains(QStringLiteral("\u1F38A"))) {
        effect = QLatin1String("confetti");
    }
    if (effect.has_value()) {
        Q_EMIT showEffect(*effect);
    }
}

#include "moc_actionshandler.cpp"
