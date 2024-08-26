// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <Quotient/util.h>

#include <QConcatenateTablesProxyModel>
#include <QQmlEngine>

#include <QPointer>
#include <Quotient/csapi/relations.h>
#include <Quotient/events/roomevent.h>
#include <Quotient/events/roommessageevent.h>
#include <deque>
#include <optional>

#include "linkpreviewer.h"
#include "messagecontentmodel.h"

class NeoChatRoom;
class ReactionModel;

/**
 * @class ThreadChatBarModel
 *
 * A model to provide a chat bar component to send new messages in a thread.
 */
class ThreadChatBarModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /**
     * @brief Defines the model roles.
     *
     * The role values need to match MessageContentModel not to blow up.
     *
     * @sa MessageContentModel
     */
    enum Roles {
        ComponentTypeRole = MessageContentModel::ComponentTypeRole, /**< The type of component to visualise the message. */
        ChatBarCacheRole = MessageContentModel::ChatBarCacheRole, /**< The ChatBarCache to use. */
    };
    Q_ENUM(Roles)

    explicit ThreadChatBarModel(QObject *parent, NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief 1 or 0, depending on whether a chat bar should be shown.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a map with ComponentTypeRole it's the only one.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    QPointer<NeoChatRoom> m_room;
};

/**
 * @class ThreadModel
 *
 * This class defines the model for visualising a thread.
 *
 * The class also provides functions to access the data of the root event, typically
 * used to visualise the thread in a list of room threads.
 */
class ThreadModel : public QConcatenateTablesProxyModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    explicit ThreadModel(const QString &threadRootId, NeoChatRoom *room);

    QString threadRootId() const;

    /**
     * @brief The content model for the thread root event.
     */
    MessageContentModel *threadRootContentModel() const;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Whether there is more data available for the model to fetch.
     *
     * @sa QAbstractItemModel::canFetchMore()
     */
    bool canFetchMore(const QModelIndex &parent) const override;

    /**
     * @brief Fetches the next batch of model data if any is available.
     *
     * @sa QAbstractItemModel::fetchMore()
     */
    void fetchMore(const QModelIndex &parent) override;

private:
    QString m_threadRootId;

    std::unique_ptr<MessageContentModel> m_threadRootContentModel;

    std::deque<MessageContentModel *> m_contentModels;
    ThreadChatBarModel *m_threadChatBarModel;

    QList<QString> m_events;
    QList<QString> m_pendingEvents;

    std::unordered_map<QString, std::unique_ptr<Quotient::RoomEvent>> m_unloadedEvents;

    QMap<QString, QSharedPointer<ReactionModel>> m_reactionModels;

    QPointer<Quotient::GetRelatingEventsWithRelTypeJob> m_currentJob = nullptr;
    std::optional<QString> m_nextBatch = QString();
    bool m_addingPending = false;

    void addNewEvent(const Quotient::RoomEvent *event);
    void addModels();
    void clearModels();
};
