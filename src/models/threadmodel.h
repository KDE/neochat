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
