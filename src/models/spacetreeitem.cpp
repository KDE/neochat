// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "spacetreeitem.h"

#include "neochatconnection.h"

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

SpaceTreeItem::~SpaceTreeItem()
{
    qDeleteAll(m_children);
}

SpaceTreeItem *SpaceTreeItem::child(int number)
{
    if (number < 0 || number >= m_children.size()) {
        return nullptr;
    }
    return m_children[number];
}

int SpaceTreeItem::childCount() const
{
    return m_children.count();
}

bool SpaceTreeItem::insertChild(int row, SpaceTreeItem *newChild)
{
    if (row < 0 || row > m_children.size()) {
        return false;
    }
    m_children.insert(row, newChild);
    return true;
}

bool SpaceTreeItem::removeChild(int row)
{
    if (row < 0 || row >= m_children.size()) {
        return false;
    }
    delete m_children.takeAt(row);
    return true;
}

int SpaceTreeItem::row() const
{
    if (m_parentItem) {
        return m_parentItem->m_children.indexOf(const_cast<SpaceTreeItem *>(this));
    }
    return 0;
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
    if (m_avatarUrl.isEmpty() || m_avatarUrl.scheme() != QLatin1String("mxc")) {
        return {};
    }
    auto url = m_connection->makeMediaUrl(m_avatarUrl);
    if (url.scheme() == QLatin1String("mxc")) {
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
    return childStateContent.value(QLatin1String("suggested")).toBool();
}
