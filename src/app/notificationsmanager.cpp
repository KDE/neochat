// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "notificationsmanager.h"

#include <memory>

#include <QGuiApplication>

#include <KLocalizedString>
#include <KNotification>
#include <KNotificationPermission>
#include <KNotificationReplyAction>
#include <KirigamiAddons/Components/NameUtils>

#include <QPainter>
#include <Quotient/accountregistry.h>
#include <Quotient/csapi/pushrules.h>
#include <Quotient/events/roommemberevent.h>
#include <Quotient/user.h>

#ifdef HAVE_KIO
#include <KIO/ApplicationLauncherJob>
#endif

#include "controller.h"
#include "jobs/neochatgetcommonroomsjob.h"
#include "neochatconfig.h"
#include "neochatconnection.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "texthandler.h"
#include "windowcontroller.h"

using namespace Quotient;

NotificationsManager::NotificationsManager(QObject *parent)
    : QObject(parent)
{
}

void NotificationsManager::handleNotifications(QPointer<NeoChatConnection> connection)
{
    if (KNotificationPermission::checkPermission() == Qt::PermissionStatus::Granted) {
        startNotificationJob(connection);
    } else if (!permissionAsked) {
        KNotificationPermission::requestPermission(this, [this, connection](Qt::PermissionStatus result) {
            if (result == Qt::PermissionStatus::Granted) {
                startNotificationJob(connection);
            } else {
                permissionAsked = true;
            }
        });
    }
}

void NotificationsManager::startNotificationJob(QPointer<NeoChatConnection> connection)
{
    if (connection == nullptr) {
        return;
    }

    if (!m_connActiveJob.contains(connection->user()->id())) {
        m_connActiveJob.append(connection->user()->id());
        connection->callApi<GetNotificationsJob>().onResult([this, connection](const auto &job) {
            m_connActiveJob.removeAll(connection->user()->id());
            processNotificationJob(connection, job, !m_oldNotifications.contains(connection->user()->id()));
        });
    }
}

void NotificationsManager::processNotificationJob(QPointer<NeoChatConnection> connection, Quotient::GetNotificationsJob *job, bool initialization)
{
    if (!job || !connection || !connection->isLoggedIn()) {
        return;
    }

    const auto connectionId = connection->user()->id();

    const auto notifications = job->jsonData()["notifications"_L1].toArray();
    if (initialization) {
        for (const auto &notification : notifications) {
            if (!m_initialTimestamp.contains(connectionId)) {
                m_initialTimestamp[connectionId] = notification["ts"_L1].toVariant().toLongLong();
            } else {
                qint64 timestamp = notification["ts"_L1].toVariant().toLongLong();
                if (timestamp > m_initialTimestamp[connectionId]) {
                    m_initialTimestamp[connectionId] = timestamp;
                }
            }

            auto connectionNotifications = m_oldNotifications.value(connectionId);
            connectionNotifications += notification["event"_L1]["event_id"_L1].toString();
            m_oldNotifications[connectionId] = connectionNotifications;
        }
        return;
    }

    QMap<QString, std::pair<qint64, QJsonObject>> notificationsToPost;
    for (const auto &n : notifications) {
        const auto notification = n.toObject();
        if (notification["read"_L1].toBool()) {
            continue;
        }
        auto connectionNotifications = m_oldNotifications.value(connectionId);
        if (connectionNotifications.contains(notification["event"_L1]["event_id"_L1].toString())) {
            continue;
        }
        connectionNotifications += notification["event"_L1]["event_id"_L1].toString();
        m_oldNotifications[connectionId] = connectionNotifications;

        if (!shouldPostNotification(connection, n)) {
            continue;
        }

        const auto &roomId = notification["room_id"_L1].toString();
        if (!notificationsToPost.contains(roomId) || notificationsToPost[roomId].first < notification["ts"_L1].toVariant().toLongLong()) {
            notificationsToPost[roomId] = {notification["ts"_L1].toVariant().toLongLong(), notification};
        }
    }

    for (const auto &[roomId, pair] : notificationsToPost.asKeyValueRange()) {
        const auto &notification = pair.second;
        const auto room = connection->room(roomId);
        if (!room) {
            continue;
        }
        auto sender = room->member(notification["event"_L1]["sender"_L1].toString());

        if (room->joinState() == JoinState::Invite) {
            postInviteNotification(qobject_cast<NeoChatRoom *>(room));
            continue;
        }

        QString body;
        if (notification["event"_L1]["type"_L1].toString() == "org.matrix.msc3381.poll.start"_L1) {
            body = notification["event"_L1]["content"_L1]["org.matrix.msc3381.poll.start"_L1]["question"_L1]["body"_L1].toString();
        } else if (notification["event"_L1]["type"_L1] == "m.room.encrypted"_L1) {
            const auto decrypted = connection->decryptNotification(notification);
            body = decrypted["content"_L1]["body"_L1].toString();
            if (body.isEmpty()) {
                body = i18n("Encrypted Message");
            }
        } else {
            body = notification["event"_L1]["content"_L1]["body"_L1].toString();
        }

        postNotification(dynamic_cast<NeoChatRoom *>(room),
                         room->member(sender.id()),
                         body,
                         notification["event"_L1].toObject()["event_id"_L1].toString(),
                         true,
                         pair.first);
    }
}

bool NotificationsManager::shouldPostNotification(QPointer<NeoChatConnection> connection, const QJsonValue &notification)
{
    if (connection == nullptr || !connection->isLoggedIn()) {
        return false;
    }

    auto room = connection->room(notification["room_id"_L1].toString());
    if (room == nullptr) {
        return false;
    }

    // If the room is the current room and the application is active the notification
    // should not be shown.
    // This is setup so that if the application is inactive the notification will
    // always be posted, even if the room is the current room.
    bool isCurrentRoom = RoomManager::instance().currentRoom() && room->id() == RoomManager::instance().currentRoom()->id();
    if (isCurrentRoom && QGuiApplication::applicationState() == Qt::ApplicationActive) {
        return false;
    }

    // If the notification timestamp is earlier than the initial timestamp assume
    // the notification is old and shouldn't be posted.
    qint64 timestamp = notification["ts"_L1].toDouble();
    if (timestamp < m_initialTimestamp[connection->user()->id()]) {
        return false;
    }

    if (m_notifications.contains(room->id()) && m_notifications[room->id()].first > timestamp) {
        return false;
    }

    return true;
}

void NotificationsManager::postNotification(NeoChatRoom *room,
                                            const RoomMember &member,
                                            const QString &text,
                                            const QString &replyEventId,
                                            bool canReply,
                                            qint64 timestamp)
{
    const QString roomId = room->id();

    if (auto notification = m_notifications.value(roomId).second) {
        notification->close();
    }

    auto notification = new KNotification(u"message"_s);
    m_notifications.insert(roomId, {timestamp, notification});
    connect(notification, &KNotification::closed, this, [this, roomId, notification] {
        if (m_notifications[roomId].second == notification) {
            m_notifications.remove(roomId);
        }
    });

    notification->setTitle(room->displayName());

    QString entry;
    if (room->isDirectChat()) {
        entry = text.toHtmlEscaped();
    } else {
        entry = i18n("%1: %2", member.displayName(), text.toHtmlEscaped());
    }

    notification->setText(entry);
    notification->setPixmap(createNotificationImage(member, room));

    auto defaultAction = notification->addDefaultAction(i18n("Open NeoChat in this room"));
    connect(defaultAction, &KNotificationAction::activated, this, [notification, room]() {
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
        if (!room) {
            return;
        }
        auto connection = dynamic_cast<NeoChatConnection *>(Controller::instance().accounts()->get(room->localMember().id()));
        Controller::instance().setActiveConnection(connection);
        RoomManager::instance().setConnection(connection);
        RoomManager::instance().resolveResource(room->id());
    });

    if (canReply) {
        std::unique_ptr<KNotificationReplyAction> replyAction(new KNotificationReplyAction(i18n("Reply")));
        replyAction->setPlaceholderText(i18n("Reply…"));
        connect(replyAction.get(), &KNotificationReplyAction::replied, this, [room, replyEventId](const QString &text) {
            TextHandler textHandler;
            textHandler.setData(text);
            auto content = std::make_unique<Quotient::EventContent::TextContent>(textHandler.handleSendText(), u"text/html"_s);
            EventRelation relatesTo = EventRelation::replyTo(replyEventId);
            room->post<Quotient::RoomMessageEvent>(text, MessageEventType::Text, std::move(content), relatesTo);
        });
        notification->setReplyAction(std::move(replyAction));
    }

    if (Controller::instance().accounts()->rowCount() > 1) {
        notification->setHint(u"x-kde-origin-name"_s, room->localMember().id());
    }
    notification->sendEvent();
}

void NotificationsManager::postInviteNotification(NeoChatRoom *rawRoom)
{
    QPointer room(rawRoom);

    const auto roomMemberEvent = room->currentState().get<RoomMemberEvent>(room->localMember().id());
    if (roomMemberEvent == nullptr) {
        return;
    }

    if (NeoChatConfig::rejectUnknownInvites()) {
        auto job = room->connection()->callApi<NeochatGetCommonRoomsJob>(roomMemberEvent->senderId());
        connect(job, &BaseJob::result, this, [this, job, room] {
            QJsonObject replyData = job->jsonData();
            if (replyData.contains(u"joined"_s)) {
                const bool inAnyOfOurRooms = !replyData["joined"_L1].toArray().isEmpty();
                if (inAnyOfOurRooms) {
                    doPostInviteNotification(room);
                } else {
                    room->forget();
                }
            }
        });
    } else {
        doPostInviteNotification(room);
    }
}

void NotificationsManager::doPostInviteNotification(QPointer<NeoChatRoom> room)
{
    const auto roomMemberEvent = room->currentState().get<RoomMemberEvent>(room->localMember().id());
    if (roomMemberEvent == nullptr) {
        return;
    }
    const auto sender = room->member(roomMemberEvent->senderId());

    KNotification *notification = new KNotification(u"invite"_s);
    notification->setText(i18n("%1 invited you to a room", sender.htmlSafeDisplayName()));
    notification->setTitle(room->displayName());
    notification->setPixmap(createNotificationImage(sender, room));
    auto defaultAction = notification->addDefaultAction(i18n("Open this invitation in NeoChat"));
    connect(defaultAction, &KNotificationAction::activated, this, [notification, room]() {
        if (!room) {
            return;
        }
        WindowController::instance().showAndRaiseWindow(notification->xdgActivationToken());
        notification->close();
        RoomManager::instance().resolveResource(room->id());
    });

    const auto acceptAction = notification->addAction(i18nc("@action:button The thing being accepted is an invitation to chat", "Accept"));
    const auto rejectAction = notification->addAction(i18nc("@action:button The thing being rejected is an invitation to chat", "Reject"));
    const auto rejectAndIgnoreAction =
        notification->addAction(i18nc("@action:button The thing being rejected is an invitation to chat", "Reject and Ignore User"));
    connect(acceptAction, &KNotificationAction::activated, this, [room, notification]() {
        if (!room) {
            return;
        }
        room->acceptInvitation();
        notification->close();
    });
    connect(rejectAction, &KNotificationAction::activated, this, [room, notification]() {
        if (!room) {
            return;
        }
        room->forget();
        notification->close();
    });
    connect(rejectAndIgnoreAction, &KNotificationAction::activated, this, [room, notification]() {
        if (!room) {
            return;
        }
        room->forget();
        room->connection()->addToIgnoredUsers(room->invitingUserId());
        notification->close();
    });
    connect(notification, &KNotification::closed, this, [this, room]() {
        if (!room) {
            return;
        }
        m_invitations.remove(room->id());
    });

    if (Controller::instance().accounts()->rowCount() > 1) {
        notification->setHint(u"x-kde-origin-name"_s, room->localMember().id());
    }

    notification->sendEvent();
}

void NotificationsManager::clearInvitationNotification(const QString &roomId)
{
    if (m_invitations.contains(roomId)) {
        m_invitations[roomId]->close();
    }
}

void NotificationsManager::postPushNotification(const QByteArray &message)
{
    const auto json = QJsonDocument::fromJson(message).object();

    const auto type = json["notification"_L1]["type"_L1].toString();

    // the only two types of push notifications we support right now
    if (type == u"m.room.message"_s || type == u"m.room.encrypted"_s) {
        auto notification = new KNotification("message"_L1);

        const auto sender = json["notification"_L1]["sender_display_name"_L1].toString();
        const auto roomName = json["notification"_L1]["room_name"_L1].toString();
        const auto roomId = json["notification"_L1]["room_id"_L1].toString();

        if (roomName.isEmpty() || sender == roomName) {
            notification->setTitle(sender);
        } else {
            notification->setTitle(i18n("%1 (%2)", sender, roomName));
        }

        if (type == u"m.room.message"_s) {
            const auto text = json["notification"_L1]["content"_L1]["body"_L1].toString();
            notification->setText(text.toHtmlEscaped());
        } else if (type == u"m.room.encrypted"_s) {
            notification->setText(i18n("Encrypted Message"));
        }

#ifdef HAVE_KIO
        auto openAction = notification->addAction(i18n("Open NeoChat"));
        connect(openAction, &KNotificationAction::activated, notification, [=]() {
            QString properId = roomId;
            properId = properId.replace(u"#"_s, QString());
            properId = properId.replace(u"!"_s, QString());

            auto *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopName(u"org.kde.neochat"_s));
            job->setUrls({QUrl::fromUserInput(u"matrix:r/%1"_s.arg(properId))});
            job->start();
        });
#endif

        connect(notification, &KNotification::closed, qGuiApp, &QGuiApplication::quit);

        notification->sendEvent();
    } else {
        qWarning() << "Skipping unsupported push notification" << type;
    }
}

QPixmap NotificationsManager::createNotificationImage(const Quotient::RoomMember &member, NeoChatRoom *room)
{
    QImage senderIcon = member.avatar(avatarDimension, avatarDimension, {});
    bool senderIconIsPlaceholder = false;
    if (senderIcon.isNull()) {
        senderIcon = createPlaceholderImage(member.displayName());
        senderIconIsPlaceholder = true;
    }

    QImage icon;
    if (room->isDirectChat()) {
        icon = senderIcon;
    } else {
        QImage roomIcon = room->avatar(avatarDimension, avatarDimension);
        bool roomIconIsPlaceholder = false;
        if (roomIcon.isNull()) {
            roomIcon = createPlaceholderImage(room->displayName());
            roomIconIsPlaceholder = true;
        }

        icon = createCombinedNotificationImage(senderIcon, senderIconIsPlaceholder, roomIcon, roomIconIsPlaceholder);
    }

    return QPixmap::fromImage(icon);
}

QImage NotificationsManager::createCombinedNotificationImage(const QImage &senderIcon,
                                                             const bool senderIconIsPlaceholder,
                                                             const QImage &roomIcon,
                                                             const bool roomIconIsPlaceholder)
{
    // Handle avatars that are lopsided in one dimension
    const int biggestDimension = std::max(senderIcon.width(), senderIcon.height());
    const QRectF imageRect = QRect{0, 0, biggestDimension, biggestDimension}.toRectF();

    QImage roundedImage(imageRect.size().toSize(), QImage::Format_ARGB32);
    roundedImage.fill(Qt::transparent);

    QPainter painter(&roundedImage);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    if (senderIconIsPlaceholder) {
        painter.drawImage(imageRect, senderIcon);
    } else {
        // Fill background for transparent non-placeholder avatars
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(imageRect, imageRect.width(), imageRect.height());

        painter.setBrush(senderIcon.scaledToHeight(biggestDimension));
        painter.drawRoundedRect(imageRect, imageRect.width(), imageRect.height());
    }

    const QRectF lowerQuarter{imageRect.center(), imageRect.size() / 2.0};

    if (roomIconIsPlaceholder) {
        // Ditto for room icons, but we also want to "carve out" the transparent area for readability
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.setBrush(Qt::transparent);
        painter.drawRoundedRect(lowerQuarter, lowerQuarter.width(), lowerQuarter.height());

        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(lowerQuarter, roomIcon);
    } else {
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(lowerQuarter, lowerQuarter.width(), lowerQuarter.height());

        painter.setBrush(roomIcon.scaled(lowerQuarter.size().toSize()));
        painter.drawRoundedRect(lowerQuarter, lowerQuarter.width(), lowerQuarter.height());
    }

    return roundedImage;
}

QImage NotificationsManager::createPlaceholderImage(const QString &name)
{
    const QColor color = NameUtils().colorsFromString(name);

    QImage image(avatarDimension, avatarDimension, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw background
    QColor backgroundColor = color;
    backgroundColor.setAlphaF(0.07); // Same as in Kirigami Add-ons.

    painter.setBrush(backgroundColor);
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(image.rect(), image.width(), image.height());

    constexpr float borderWidth = 3.0; // Slightly bigger than in Add-ons so it renders better with QPainter at these dimensions.

    // Draw border
    painter.setBrush(Qt::transparent);
    painter.setPen(QPen(color, borderWidth));
    painter.drawRoundedRect(image.rect().toRectF().marginsRemoved(QMarginsF(borderWidth, borderWidth, borderWidth, borderWidth)),
                            image.width(),
                            image.height());

    const QString initials = NameUtils().initialsFromString(name);

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);

    // Calculation similar to the one found in Kirigami Add-ons.
    constexpr int largeSpacing = 8; // Same as what's defined in kirigami.
    constexpr int padding = std::max(0, std::min(largeSpacing, avatarDimension - largeSpacing * 2));

    QFont font;
    font.setPixelSize((avatarDimension - padding) / 2);

    painter.setBrush(color);
    painter.setPen(color);
    painter.setFont(font);
    painter.drawText(image.rect(), initials, option);

    return image;
}

#include "moc_notificationsmanager.cpp"
