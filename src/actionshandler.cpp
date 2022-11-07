// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionshandler.h"

#include "controller.h"

#include <csapi/joining.h>
#include <events/roommemberevent.h>

#include <cmark.h>

#include <KLocalizedString>
#include <QStringBuilder>

#include "actionsmodel.h"
#include "controller.h"
#include "customemojimodel.h"
#include "neochatconfig.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "neochatuser.h"

using namespace Quotient;

QString markdownToHTML(const QString &markdown)
{
    const auto str = markdown.toUtf8();
    char *tmp_buf = cmark_markdown_to_html(str.constData(), str.size(), CMARK_OPT_HARDBREAKS);

    const std::string html(tmp_buf);

    free(tmp_buf);

    auto result = QString::fromStdString(html).trimmed();

    result.replace("<!-- raw HTML omitted -->", "");

    return result;
}

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

void ActionsHandler::handleMessage()
{
    checkEffects();
    if (!m_room->chatBoxAttachmentPath().isEmpty()) {
        auto path = m_room->chatBoxAttachmentPath();
        path = path.mid(path.lastIndexOf('/') + 1);
        m_room->uploadFile(m_room->chatBoxAttachmentPath(), m_room->chatBoxText().isEmpty() ? path : m_room->chatBoxText());
        m_room->setChatBoxAttachmentPath({});
        m_room->setChatBoxText({});
        return;
    }
    QString handledText = m_room->chatBoxText();

    std::sort(m_room->mentions()->begin(), m_room->mentions()->end(), [](const auto &a, const auto &b) -> bool {
        return a.cursor.anchor() > b.cursor.anchor();
    });

    for (const auto &mention : *m_room->mentions()) {
        if (mention.text.isEmpty() || mention.id.isEmpty()) {
            continue;
        }
        handledText = handledText.replace(mention.cursor.anchor(),
                                          mention.cursor.position() - mention.cursor.anchor(),
                                          QStringLiteral("[%1](https://matrix.to/#/%2)").arg(mention.text, mention.id));
    }
    m_room->mentions()->clear();

    if (NeoChatConfig::allowQuickEdit()) {
        QRegularExpression sed("^s/([^/]*)/([^/]*)(/g)?$");
        auto match = sed.match(m_room->chatBoxText());
        if (match.hasMatch()) {
            const QString regex = match.captured(1);
            const QString replacement = match.captured(2);
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

    handledText = markdownToHTML(handledText);
    handledText = CustomEmojiModel::instance().preprocessText(handledText);

    if (handledText.length() == 0) {
        return;
    }

    m_room->postMessage(m_room->chatBoxText(), handledText, messageType, m_room->chatBoxReplyId(), m_room->chatBoxEditId());
}

void ActionsHandler::checkEffects()
{
    std::optional<QString> effect = std::nullopt;
    const auto &text = m_room->chatBoxText();
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
