// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <Quotient/csapi/space_hierarchy.h>

class NeoChatConnection;

/**
 * @class SpaceTreeItem
 *
 * This class defines an item in the space tree hierarchy model.
 *
 * @note This is separate from Quotient::Room and NeoChatRoom because we don't have
 *       full room information for any room/space the user hasn't joined and we
 *       don't want to create one for ever possible child in a space as that would
 *       be expensive.
 *
 * @sa Quotient::Room, NeoChatRoom
 */
class SpaceTreeItem
{
public:
    explicit SpaceTreeItem(NeoChatConnection *connection,
                           SpaceTreeItem *parent = nullptr,
                           const QString &id = {},
                           const QString &name = {},
                           const QString &canonicalAlias = {},
                           const QString &topic = {},
                           int memberCount = {},
                           const QUrl &avatarUrl = {},
                           bool allowGuests = {},
                           bool worldReadable = {},
                           bool isSpace = {});
    ~SpaceTreeItem();

    /**
     * @brief Return the child at the given row number.
     *
     * Nullptr is returned if there is no child at the given row number.
     */
    SpaceTreeItem *child(int number);

    /**
     * @brief The number of children this item has.
     */
    int childCount() const;

    /**
     * @brief Insert the given child at the given row number.
     */
    bool insertChild(int row, SpaceTreeItem *newChild);

    /**
     * @brief Remove the child at the given row number.
     *
     * @return True if a child was removed, false if the given row isn't valid.
     */
    bool removeChild(int row);

    /**
     * @brief Return this item's parent.
     */
    SpaceTreeItem *parentItem();

    /**
     * @brief Return the row number for this child relative to the parent.
     *
     * @return The row value if the child has a parent, 0 otherwise.
     */
    int row() const;

    /**
     * @brief The ID of the room.
     */
    QString id() const;

    /**
     * @brief The name of the room, if any.
     */
    QString name() const;

    /**
     * @brief The canonical alias of the room, if any.
     */
    QString canonicalAlias() const;

    /**
     * @brief The topic of the room, if any.
     */
    QString topic() const;

    /**
     * @brief The number of members joined to the room.
     */
    int memberCount() const;

    /**
     * @brief The URL for the room's avatar, if one is set.
     *
     * @return A CS API QUrl.
     */
    QUrl avatarUrl() const;

    /**
     * @brief Whether guest users may join the room and participate in it.
     *
     * If they can, they will be subject to ordinary power level rules like any other users.
     */
    bool allowGuests() const;

    /**
     * @brief Whether the room may be viewed by guest users without joining.
     */
    bool worldReadable() const;

    /**
     * @brief Whether the local user is a member of the rooom.
     */
    bool isJoined() const;

    /**
     * @brief Whether the room is a space.
     */
    bool isSpace() const;

private:
    NeoChatConnection *m_connection;
    QList<SpaceTreeItem *> m_children;
    SpaceTreeItem *m_parentItem;

    QString m_id;
    QString m_name;
    QString m_canonicalAlias;
    QString m_topic;
    int m_memberCount;
    QUrl m_avatarUrl;
    bool m_allowGuests;
    bool m_worldReadable;
    bool m_isSpace;
};
