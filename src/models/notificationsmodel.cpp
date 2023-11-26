// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "notificationsmodel.h"

#include <Quotient/connection.h>
#include <Quotient/events/event.h>
#include <Quotient/uri.h>

#include "eventhandler.h"
#include "neochatroom.h"

using namespace Quotient;

NotificationsModel::NotificationsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int NotificationsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_notifications.count();
}

QVariant NotificationsModel::data(const QModelIndex &index, int role) const
{
    auto row = index.row();
    if (row < 0 || row >= m_notifications.count()) {
        return {};
    }
    if (role == TextRole) {
        return m_notifications[row].text;
    }
    if (role == RoomIdRole) {
        return m_notifications[row].roomId;
    }
    if (role == AuthorName) {
        return m_notifications[row].authorName;
    }
    if (role == AuthorAvatar) {
        return m_notifications[row].authorAvatar;
    }
    if (role == RoomRole) {
        return QVariant::fromValue(m_connection->room(m_notifications[row].roomId));
    }
    if (role == EventIdRole) {
        return m_notifications[row].eventId;
    }
    if (role == RoomDisplayNameRole) {
        return m_notifications[row].roomDisplayName;
    }
    if (role == UriRole) {
        return Uri(m_notifications[row].roomId.toLatin1(), m_notifications[row].eventId.toLatin1()).toUrl();
    }
    return {};
}

QHash<int, QByteArray> NotificationsModel::roleNames() const
{
    return {
        {TextRole, "text"},
        {RoomIdRole, "roomId"},
        {AuthorName, "authorName"},
        {AuthorAvatar, "authorAvatar"},
        {RoomRole, "room"},
        {EventIdRole, "eventId"},
        {RoomDisplayNameRole, "roomDisplayName"},
        {UriRole, "uri"},
    };
}

NeoChatConnection *NotificationsModel::connection() const
{
    return m_connection;
}

void NotificationsModel::setConnection(NeoChatConnection *connection)
{
    if (m_connection) {
        // disconnect things...
    }
    if (!connection) {
        return;
    }
    m_connection = connection;
    Q_EMIT connectionChanged();
    connect(connection, &Connection::syncDone, this, [=]() {
        loadData();
    });
    loadData();
}

void NotificationsModel::loadData()
{
    Q_ASSERT(m_connection);
    if (m_job || (m_notifications.size() && m_nextToken.isEmpty())) {
        return;
    }
    m_job = m_connection->callApi<GetNotificationsJob>(m_nextToken);
    Q_EMIT loadingChanged();
    connect(m_job, &BaseJob::finished, this, [this]() {
        m_nextToken = m_job->nextToken();
        Q_EMIT nextTokenChanged();
        for (const auto &notification : m_job->notifications()) {
            if (std::any_of(notification.actions.constBegin(), notification.actions.constEnd(), [](const QVariant &it) {
                    if (it.canConvert<QVariantMap>()) {
                        auto map = it.toMap();
                        if (map["set_tweak"_ls] == "highlight"_ls) {
                            return true;
                        }
                    }
                    return false;
                })) {
                const auto &authorId = notification.event->fullJson()["sender"_ls].toString();
                const auto &room = m_connection->room(notification.roomId);
                if (!room) {
                    continue;
                }
                auto u = room->memberAvatarUrl(authorId);
                auto avatar = u.isEmpty() ? QUrl() : connection()->makeMediaUrl(u);
                const auto &authorAvatar = avatar.isValid() && avatar.scheme() == QStringLiteral("mxc") ? avatar : QUrl();

                const auto &roomEvent = eventCast<const RoomEvent>(notification.event.get());
                EventHandler eventHandler;
                eventHandler.setRoom(dynamic_cast<NeoChatRoom *>(room));
                eventHandler.setEvent(roomEvent);
                beginInsertRows({}, m_notifications.length(), m_notifications.length());
                m_notifications += Notification{
                    .roomId = notification.roomId,
                    .text = room->htmlSafeMemberName(authorId) + (roomEvent->is<StateEvent>() ? QStringLiteral(" ") : QStringLiteral(": "))
                        + eventHandler.getPlainBody(true),
                    .authorName = room->htmlSafeMemberName(authorId),
                    .authorAvatar = authorAvatar,
                    .eventId = roomEvent->id(),
                    .roomDisplayName = room->displayName(),
                };
                endInsertRows();
            }
        }
        m_job = nullptr;
        Q_EMIT loadingChanged();
    });
}

bool NotificationsModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return !m_nextToken.isEmpty();
}

void NotificationsModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    loadData();
}

bool NotificationsModel::loading() const
{
    return m_job;
}

QString NotificationsModel::nextToken() const
{
    return m_nextToken;
}
