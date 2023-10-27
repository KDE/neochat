// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/csapi/threads_list.h>
#include <Quotient/events/roomevent.h>
#include <Quotient/events/roommessageevent.h>

#include "neochatroom.h"
#include "threadmodel.h"

/**
 * @class RoomThreadsModel
 *
 * This class defines the model for visualising a thread.
 */
class RoomThreadsModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current room that the model is getting its messages from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

public:
    /**
     * @brief Defines the model roles.
     *
     * For documentation of the roles, see MessageEventModel.
     *
     * Some of the roles exist only for compatibility with the MessageEventModel,
     * since the same delegates are used.
     */
    enum Roles {
        DisplayRole = Qt::DisplayRole,
        EventIdRole,
        AuthorRole,
        TimeRole,
        TimeStringRole,
        ThreadModelRole,
    };
    Q_ENUM(Roles)

    explicit RoomThreadsModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

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

Q_SIGNALS:
    void roomChanged();

private:
    NeoChatRoom *m_room;
    QVector<ThreadModel *> m_threadModels;

    QPointer<Quotient::GetThreadRootsJob> m_currentJob = nullptr;
    Quotient::Omittable<QString> m_nextBatch = QString();

    void initializeModel();
};
