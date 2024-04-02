// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomtreeitem.h"

RoomTreeItem::RoomTreeItem(TreeData data, RoomTreeItem *parent)
    : m_parentItem(parent)
    , m_data(data)
{
}

bool RoomTreeItem::operator==(const RoomTreeItem &other) const
{
    if (std::holds_alternative<NeoChatRoomType::Types>(m_data) && std::holds_alternative<NeoChatRoomType::Types>(other.data())) {
        return std::get<NeoChatRoomType::Types>(m_data) == std::get<NeoChatRoomType::Types>(m_data);
    }
    if (std::holds_alternative<NeoChatRoom *>(m_data) && std::holds_alternative<NeoChatRoom *>(other.data())) {
        return std::get<NeoChatRoom *>(m_data)->id() == std::get<NeoChatRoom *>(m_data)->id();
    }
    return false;
}

RoomTreeItem *RoomTreeItem::child(int row)
{
    return row >= 0 && row < childCount() ? m_children.at(row).get() : nullptr;
}

int RoomTreeItem::childCount() const
{
    return int(m_children.size());
}

bool RoomTreeItem::insertChild(std::unique_ptr<RoomTreeItem> newChild)
{
    if (newChild == nullptr) {
        return false;
    }

    for (auto it = m_children.begin(), end = m_children.end(); it != end; ++it) {
        if (*it == newChild) {
            *it = std::move(newChild);
            return true;
        }
    }

    m_children.push_back(std::move(newChild));
    return true;
}

bool RoomTreeItem::removeChild(int row)
{
    if (row < 0 || row >= childCount()) {
        return false;
    }
    m_children.erase(m_children.begin() + row);
    return true;
}

int RoomTreeItem::row() const
{
    if (m_parentItem == nullptr) {
        return 0;
    }

    const auto it = std::find_if(m_parentItem->m_children.cbegin(), m_parentItem->m_children.cend(), [this](const std::unique_ptr<RoomTreeItem> &treeItem) {
        return treeItem.get() == this;
    });

    if (it != m_parentItem->m_children.cend()) {
        return std::distance(m_parentItem->m_children.cbegin(), it);
    }
    Q_ASSERT(false); // should not happen
    return -1;
}

RoomTreeItem *RoomTreeItem::parentItem() const
{
    return m_parentItem;
}

RoomTreeItem::TreeData RoomTreeItem::data() const
{
    return m_data;
}

std::optional<int> RoomTreeItem::rowForRoom(Quotient::Room *room) const
{
    Q_ASSERT_X(std::holds_alternative<NeoChatRoomType::Types>(m_data), __FUNCTION__, "rowForRoom only works items for rooms not categories");

    int i = 0;
    for (const auto &child : m_children) {
        if (std::get<NeoChatRoom *>(child->data()) == room) {
            return i;
        }
        i++;
    }

    return std::nullopt;
}
