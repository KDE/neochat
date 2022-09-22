// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "collapsestateproxymodel.h"
#include "messageeventmodel.h"

#include <KLocalizedString>

bool CollapseStateProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    return sourceModel()->data(sourceModel()->index(source_row, 0), MessageEventModel::EventTypeRole)
        != QLatin1String("state") // If this is not a state, show it
        || sourceModel()->data(sourceModel()->index(source_row + 1, 0), MessageEventModel::EventTypeRole)
        != QLatin1String("state") // If this is the first state in a block, show it. TODO hidden events?
        || sourceModel()->data(sourceModel()->index(source_row, 0), MessageEventModel::ShowSectionRole).toBool() // If it's a new day, show it
        || sourceModel()->data(sourceModel()->index(source_row, 0), MessageEventModel::EventResolvedTypeRole)
        != sourceModel()->data(sourceModel()->index(source_row + 1, 0),
                               MessageEventModel::EventResolvedTypeRole) // Also show it if it's of a different type than the one before TODO improve in
        || sourceModel()->data(sourceModel()->index(source_row, 0), MessageEventModel::AuthorIdRole)
        != sourceModel()->data(sourceModel()->index(source_row + 1, 0), MessageEventModel::AuthorIdRole); // Also show it if it's a different author
}

QVariant CollapseStateProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == AggregateDisplayRole) {
        return aggregateEventToString(mapToSource(index).row());
    }
    return sourceModel()->data(mapToSource(index), role);
}

QHash<int, QByteArray> CollapseStateProxyModel::roleNames() const
{
    auto roles = sourceModel()->roleNames();
    roles[AggregateDisplayRole] = "aggregateDisplay";
    return roles;
}

QString CollapseStateProxyModel::aggregateEventToString(int sourceRow) const
{
    QStringList parts;
    for (int i = sourceRow; i >= 0; i--) {
        parts += sourceModel()->data(sourceModel()->index(i, 0), Qt::DisplayRole).toString();
        if (sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::EventTypeRole) != QLatin1String("state") // If it's not a state event
            || (i > 0
                && sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::EventResolvedTypeRole)
                    != sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::EventResolvedTypeRole)) // or of a different type
            || (i > 0
                && sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorIdRole)
                    != sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::AuthorIdRole)) // or by a different author
            || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
        ) {
            break;
        }
    }
    if (!parts.isEmpty()) {
        QStringList chunks;
        while (!parts.isEmpty()) {
            chunks += QString();
            int count = 1;
            auto part = parts.takeFirst();
            chunks.last() += part;
            while (!parts.isEmpty() && parts.first() == part) {
                parts.removeFirst();
                count++;
            }
            if (count > 1) {
                chunks.last() += i18ncp("[user did something] n times", " %1 time", " %1 times", count);
            }
        }
        QString text = chunks.takeFirst();

        if (chunks.size() > 0) {
            while (chunks.size() > 1) {
                text += i18nc("[action 1], [action 2 and action 3]", ", ");
                text += chunks.takeFirst();
            }
            text += i18nc("[action 1, action 2] and [action 3]", " and ");
            text += chunks.takeFirst();
        }
        return text;
    } else {
        return {};
    }
}
