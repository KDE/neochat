// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "actionshandler.h"

#include "chatbarcache.h"
#include "models/actionsmodel.h"
#include "neochatconfig.h"
#include "texthandler.h"

using namespace Quotient;
using namespace Qt::StringLiterals;

void ActionsHandler::handleMessageEvent(NeoChatRoom *room, ChatBarCache *chatBarCache)
{
    if (room == nullptr || chatBarCache == nullptr) {
        qWarning() << "ActionsHandler::handleMessageEvent - called with m_room and/or chatBarCache set to nullptr.";
        return;
    }

    if (!chatBarCache->attachmentPath().isEmpty()) {
        QUrl url(chatBarCache->attachmentPath());
        auto path = url.isLocalFile() ? url.toLocalFile() : url.toString();
        room->uploadFile(QUrl(path), chatBarCache->text().isEmpty() ? path.mid(path.lastIndexOf(u'/') + 1) : chatBarCache->text());
        chatBarCache->setAttachmentPath({});
        chatBarCache->setText({});
        return;
    }

    const auto handledText = handleMentions(chatBarCache);
    const auto result = handleQuickEdit(room, handledText);
    if (!result) {
        handleMessage(room, handledText, chatBarCache);
    }
}

QString ActionsHandler::handleMentions(ChatBarCache *chatBarCache)
{
    const auto mentions = chatBarCache->mentions();
    std::sort(mentions->begin(), mentions->end(), [](const auto &a, const auto &b) -> bool {
        return a.cursor.anchor() > b.cursor.anchor();
    });

    auto handledText = chatBarCache->text();
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

bool ActionsHandler::handleQuickEdit(NeoChatRoom *room, const QString &handledText)
{
    if (room == nullptr) {
        return false;
    }

    if (NeoChatConfig::allowQuickEdit()) {
        QRegularExpression sed(QStringLiteral("^s/([^/]*)/([^/]*)(/g)?$"));
        auto match = sed.match(handledText);
        if (match.hasMatch()) {
            const QString regex = match.captured(1);
            const QString replacement = match.captured(2).toHtmlEscaped();
            const QString flags = match.captured(3);

            for (auto it = room->messageEvents().crbegin(); it != room->messageEvents().crend(); it++) {
                if (const auto event = eventCast<const RoomMessageEvent>(&**it)) {
                    if (event->senderId() == room->localMember().id() && event->hasTextContent()) {
                        QString originalString;
                        if (event->content()) {
#if Quotient_VERSION_MINOR > 8
                            originalString = static_cast<const Quotient::EventContent::TextContent *>(event->content().get())->body;
#else
                            originalString = static_cast<const Quotient::EventContent::TextContent *>(event->content())->body;
#endif
                        } else {
                            originalString = event->plainBody();
                        }
                        if (flags == "/g"_L1) {
                            room->postHtmlMessage(handledText, originalString.replace(regex, replacement), event->msgtype(), {}, event->id());
                        } else {
                            room->postHtmlMessage(handledText,
                                                  originalString.replace(originalString.indexOf(regex), regex.size(), replacement),
                                                  event->msgtype(),
                                                  {},
                                                  event->id());
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void ActionsHandler::handleMessage(NeoChatRoom *room, QString handledText, ChatBarCache *chatBarCache)
{
    if (room == nullptr) {
        return;
    }

    auto messageType = RoomMessageEvent::MsgType::Text;

    if (handledText.startsWith(QLatin1Char('/'))) {
        for (const auto &action : ActionsModel::instance().allActions()) {
            if (handledText.indexOf(action.prefix) == 1
                && (handledText.indexOf(" "_ls) == action.prefix.length() + 1 || handledText.length() == action.prefix.length() + 1)) {
                handledText = action.handle(handledText.mid(action.prefix.length() + 1).trimmed(), room, chatBarCache);
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

    if (handledText.length() == 0) {
        return;
    }

    room->postMessage(chatBarCache->text(), handledText, messageType, chatBarCache->replyId(), chatBarCache->editId(), chatBarCache->threadId());
}

#include "moc_actionshandler.cpp"
