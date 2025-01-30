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

/**
 * @class ThreadFetchModel
 *
 * A model to provide a fetch more historical messages button in a thread.
 */
class ThreadFetchModel : public QAbstractListModel
{
    Q_OBJECT

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
    };
    Q_ENUM(Roles)

    explicit ThreadFetchModel(QObject *parent);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief 1 or 0, depending on whether there are more messages to download.
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
};

/**
 * @class ThreadChatBarModel
 *
 * A model to provide a chat bar component to send new messages in a thread.
 */
class ThreadChatBarModel : public QAbstractListModel
{
    Q_OBJECT

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
        ThreadRootRole = MessageContentModel::ThreadRootRole, /**< The thread root event ID for the thread. */
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
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Whether there are more events for the model to fetch.
     */
    bool moreEventsAvailable(const QModelIndex &parent) const;

    /**
     * @brief Fetches the next batch of events if any is available.
     */
    Q_INVOKABLE void fetchMoreEvents(int max = 5);

    /**
     * @brief Close the link preview at the given index.
     *
     * If the given index is not a link preview component, nothing happens.
     */
    Q_INVOKABLE void closeLinkPreview(int row);

Q_SIGNALS:
    void moreEventsAvailableChanged();

private:
    QString m_threadRootId;
    QPointer<MessageContentModel> m_threadRootContentModel;

    std::deque<QString> m_events;
    ThreadFetchModel *m_threadFetchModel;
    ThreadChatBarModel *m_threadChatBarModel;

    QPointer<Quotient::GetRelatingEventsWithRelTypeJob> m_currentJob = nullptr;
    std::optional<QString> m_nextBatch = QString();
    bool m_addingPending = false;

    void checkPending();
    void addNewEvent(const Quotient::RoomEvent *event);
    void addModels();
    void clearModels();
};
