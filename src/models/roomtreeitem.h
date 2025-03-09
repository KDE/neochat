
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "neochatroomtype.h"

namespace sdk
{
struct RoomListRoom;
}

struct RoomWrapper {
    std::optional<rust::Box<sdk::RoomListRoom>> item;
};

/**
 * @class RoomTreeItem
 *
 * This class defines an item in a room tree.
 */
class RoomTreeItem
{
public:
    using TreeData = std::variant<RoomWrapper *, NeoChatRoomType::Type>;

    explicit RoomTreeItem(TreeData data, RoomTreeItem *parent = nullptr);

    bool operator==(const RoomTreeItem &other) const;

    /**
     * @brief Return the child at the given row number.
     *
     * Nullptr is returned if there is no child at the given row number.
     */
    RoomTreeItem *child(int row);

    /**
     * @brief The number of children this item has.
     */
    int childCount() const;

    /**
     * @brief Insert the given child.
     */
    bool insertChild(std::unique_ptr<RoomTreeItem> newChild);

    /**
     * @brief Remove the child at the given row number.
     *
     * @return True if a child was removed, false if the given row isn't valid.
     */
    bool removeChild(int row);

    /**
     * @brief Return this item's parent.
     */
    RoomTreeItem *parentItem() const;

    /**
     * @brief Return the row number for this child relative to the parent.
     *
     * @return The row value if the child has a parent, 0 otherwise.
     */
    int row() const;

    /**
     * @brief Return this item's data.
     */
    TreeData data() const;

    std::optional<int> rowForRoom(rust::Box<sdk::RoomListRoom> room) const;

private:
    std::vector<std::unique_ptr<RoomTreeItem>> m_children;
    RoomTreeItem *m_parentItem;

    TreeData m_data;
};
