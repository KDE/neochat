// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QConcatenateTablesProxyModel>
#include <QQmlEngine>

#include "messageeventmodel.h"
#include "neochatroom.h"

/**
 * @class TimelineEndModel
 *
 * A model to provide a single delegate to mark the end of the timeline.
 *
 * The delegate will either be a loading delegate if more events are being loaded
 * or a timeline end delegate if all history is loaded.
 */
class TimelineEndModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        DelegateTypeRole = MessageEventModel::DelegateTypeRole, /**< The delegate type of the message. */
    };
    Q_ENUM(Roles)

    explicit TimelineEndModel(QObject *parent = nullptr);

    /**
     * @brief Set the room for the timeline.
     */
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief 1, the answer is always 1.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a map with DelegateTypeRole it's the only one.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    NeoChatRoom *m_room = nullptr;
};

/**
 * @class TimelineModel
 *
 * A model to visualise a room timeline.
 *
 * This model combines a MessageEventModel with a TimelineEndModel.
 *
 * @sa MessageEventModel, TimelineEndModel
 */
class TimelineModel : public QConcatenateTablesProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current room that the model is getting its messages from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief The MessageEventModel for the timeline.
     */
    Q_PROPERTY(MessageEventModel *messageEventModel READ messageEventModel CONSTANT)

public:
    TimelineModel(QObject *parent = nullptr);

    [[nodiscard]] NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    MessageEventModel *messageEventModel() const;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractProxyModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void roomChanged();

private:
    MessageEventModel *m_messageEventModel = nullptr;
    TimelineEndModel *m_timelineEndModel = nullptr;
};
