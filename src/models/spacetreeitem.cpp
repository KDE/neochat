// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "spacetreeitem.h"

#include "neochatconnection.h"

using namespace Qt::StringLiterals;

SpaceTreeItem::SpaceTreeItem(NeoChatConnection *connection,
                             SpaceTreeItem *parent,
                             const QString &id,
                             const QString &name,
                             const QString &canonicalAlias,
                             const QString &topic,
                             int memberCount,
                             const QUrl &avatarUrl,
                             bool allowGuests,
                             bool worldReadable,
                             bool isSpace,
                             Quotient::StateEvents childStates)
    : m_connection(connection)
    , m_parentItem(parent)
    , m_id(id)
    , m_name(name)
    , m_canonicalAlias(canonicalAlias)
    , m_topic(topic)
    , m_memberCount(memberCount)
    , m_avatarUrl(avatarUrl)
    , m_allowGuests(allowGuests)
    , m_worldReadable(worldReadable)
    , m_isSpace(isSpace)
    , m_childStates(std::move(childStates))
{
}

bool SpaceTreeItem::operator==(const SpaceTreeItem &other) const
{
    return m_id == other.id();
}

SpaceTreeItem *SpaceTreeItem::child(int row)
{
    return row >= 0 && row < childCount() ? m_children.at(row).get() : nullptr;
}

int SpaceTreeItem::childCount() const
{
    return int(m_children.size());
}

bool SpaceTreeItem::insertChild(std::unique_ptr<SpaceTreeItem> newChild)
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

bool SpaceTreeItem::removeChild(int row)
{
    if (row < 0 || row >= childCount()) {
        return false;
    }
    m_children.erase(m_children.begin() + row);
    return true;
}

int SpaceTreeItem::row() const
{
    if (m_parentItem == nullptr) {
        return 0;
    }

    const auto it = std::find_if(m_parentItem->m_children.cbegin(), m_parentItem->m_children.cend(), [this](const std::unique_ptr<SpaceTreeItem> &treeItem) {
        return treeItem.get() == this;
    });

    if (it != m_parentItem->m_children.cend()) {
        return std::distance(m_parentItem->m_children.cbegin(), it);
    }
    Q_ASSERT(false); // should not happen
    return -1;
}

SpaceTreeItem *SpaceTreeItem::parentItem() const
{
    return m_parentItem;
}

QString SpaceTreeItem::id() const
{
    return m_id;
}

QString SpaceTreeItem::name() const
{
    return m_name;
}

QString SpaceTreeItem::canonicalAlias() const
{
    return m_canonicalAlias;
}

QString SpaceTreeItem::topic() const
{
    return m_topic;
}

int SpaceTreeItem::memberCount() const
{
    return m_memberCount;
}

QUrl SpaceTreeItem::avatarUrl() const
{
    if (m_avatarUrl.isEmpty() || m_avatarUrl.scheme() != u"mxc"_s) {
        return {};
    }
    auto url = m_connection->makeMediaUrl(m_avatarUrl);
    if (url.scheme() == u"mxc"_s) {
        return url;
    }
    return {};
}

bool SpaceTreeItem::allowGuests() const
{
    return m_allowGuests;
}

bool SpaceTreeItem::worldReadable() const
{
    return m_worldReadable;
}

bool SpaceTreeItem::isJoined() const
{
    if (!m_connection) {
        return false;
    }
    return m_connection->room(id(), Quotient::JoinState::Join) != nullptr;
}

bool SpaceTreeItem::isSpace() const
{
    return m_isSpace;
}

QJsonObject SpaceTreeItem::childState(const SpaceTreeItem *child) const
{
    if (child == nullptr) {
        return {};
    }
    if (child->parentItem() != this) {
        return {};
    }
    for (const auto &childState : m_childStates) {
        if (childState->stateKey() == child->id()) {
            return childState->fullJson();
        }
    }
    return {};
}

QJsonObject SpaceTreeItem::childStateContent(const SpaceTreeItem *child) const
{
    if (child == nullptr) {
        return {};
    }
    if (child->parentItem() != this) {
        return {};
    }
    for (const auto &childState : m_childStates) {
        if (childState->stateKey() == child->id()) {
            return childState->contentJson();
        }
    }
    return {};
}

void SpaceTreeItem::setChildStates(Quotient::StateEvents childStates)
{
    m_childStates.clear();
    m_childStates = std::move(childStates);
}

bool SpaceTreeItem::isSuggested() const
{
    if (m_parentItem == nullptr) {
        return false;
    }
    const auto childStateContent = m_parentItem->childStateContent(this);
    return childStateContent.value("suggested"_L1).toBool();
}
