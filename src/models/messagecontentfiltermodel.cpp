// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentfiltermodel.h"
#include "enums/messagecomponenttype.h"
#include "models/messagecontentmodel.h"

MessageContentFilterModel::MessageContentFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool MessageContentFilterModel::showAuthor() const
{
    return m_showAuthor;
}

void MessageContentFilterModel::setShowAuthor(bool showAuthor)
{
    if (showAuthor == m_showAuthor) {
        return;
    }

    m_showAuthor = showAuthor;
    Q_EMIT showAuthorChanged();
}

bool MessageContentFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (m_showAuthor) {
        return true;
    }

    const auto index = sourceModel()->index(source_row, 0, source_parent);
    auto contentType = static_cast<MessageComponentType::Type>(index.data(MessageContentModel::ComponentTypeRole).toInt());
    return contentType != MessageComponentType::Author;
}
