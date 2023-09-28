// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractItemModel>
#include <QQmlEngine>

#include <Quotient/csapi/space_hierarchy.h>
#include <qtmetamacros.h>

#include "neochatroom.h"
#include "spacetreeitem.h"

/**
 * @class SpaceChildrenModel
 *
 * Create a model that contains a list of the child rooms for any given space id.
 */
class SpaceChildrenModel : public QAbstractItemModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The current space that the hierarchy is being generated for.
     */
    Q_PROPERTY(NeoChatRoom *space READ space WRITE setSpace NOTIFY spaceChanged)

    /**
     * @brief Whether the model is loading the initial set of children.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    enum Roles {
        DisplayNameRole = Qt::DisplayRole,
        AvatarUrlRole,
        TopicRole,
        RoomIDRole,
        AliasRole,
        MemberCountRole,
        AllowGuestsRole,
        WorldReadableRole,
        IsJoinedRole,
        IsSpaceRole,
        CanAddChildrenRole,
    };

    explicit SpaceChildrenModel(QObject *parent = nullptr);
    ~SpaceChildrenModel();

    NeoChatRoom *space() const;
    void setSpace(NeoChatRoom *space);

    bool loading() const;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role = DisplayNameRole) const override;

    /**
     * @brief Returns the index of the item in the model specified by the given row, column and parent index.
     *
     * @sa QAbstractItemModel::index
     */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns the parent of the model item with the given index.
     *
     * If the item has no parent, an invalid QModelIndex is returned.
     *
     * @sa QAbstractItemModel::parent
     */
    QModelIndex parent(const QModelIndex &index) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Number of columns in the model.
     *
     * @sa QAbstractItemModel::columnCount
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Whether the room has been replaced.
     *
     * @note This information is only available if the local user is either a member
     *       of the replaced room or is a member of the successor room as currently
     *       there is no other way to obtain the required information.
     */
    bool isRoomReplaced(const QString &roomId) const;

    /**
     * @brief Add the name of new child room that is expected to be added soon.
     *
     * A pending child is one where Quotient::Connection::createRoom has been called
     * but the room hasn't synced with the server yet. This list is used to check
     * whether a new room loading should trigger a refresh of the model, as we only
     * want to trigger a refresh if the loading room is part of this space.
     */
    Q_INVOKABLE void addPendingChild(const QString &childName);

Q_SIGNALS:
    void spaceChanged();
    void loadingChanged();

private:
    NeoChatRoom *m_space = nullptr;
    SpaceTreeItem *m_rootItem;

    bool m_loading = false;
    QList<QPointer<Quotient::GetSpaceHierarchyJob>> m_currentJobs;
    QList<QString> m_pendingChildren;

    QList<QString> m_replacedRooms;

    SpaceTreeItem *getItem(const QModelIndex &index) const;

    void refreshModel();
    void insertChildren(std::vector<Quotient::GetSpaceHierarchyJob::ChildRoomsChunk> children, const QModelIndex &parent = QModelIndex());
};
