// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "collapsestateproxymodel.h"
#include "messageeventmodel.h"

#include <KLocalizedString>

bool CollapseStateProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    return sourceModel()->data(sourceModel()->index(source_row, 0), MessageEventModel::DelegateTypeRole)
        != MessageEventModel::DelegateType::State // If this is not a state, show it
        || sourceModel()->data(sourceModel()->index(source_row + 1, 0), MessageEventModel::DelegateTypeRole)
        != MessageEventModel::DelegateType::State // If this is the first state in a block, show it. TODO hidden events?
        || sourceModel()->data(sourceModel()->index(source_row, 0), MessageEventModel::ShowSectionRole).toBool(); // If it's a new day, show it
}

QVariant CollapseStateProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == AggregateDisplayRole) {
        return aggregateEventToString(mapToSource(index).row());
    } else if (role == StateEventsRole) {
        return stateEventsList(mapToSource(index).row());
    } else if (role == AuthorListRole) {
        return authorList(mapToSource(index).row());
    } else if (role == ExcessAuthorsRole) {
        return excessAuthors(mapToSource(index).row());
    }
    return sourceModel()->data(mapToSource(index), role);
}

QHash<int, QByteArray> CollapseStateProxyModel::roleNames() const
{
    auto roles = sourceModel()->roleNames();
    roles[AggregateDisplayRole] = "aggregateDisplay";
    roles[StateEventsRole] = "stateEvents";
    roles[AuthorListRole] = "authorList";
    roles[ExcessAuthorsRole] = "excessAuthors";
    return roles;
}

QString CollapseStateProxyModel::aggregateEventToString(int sourceRow) const
{
    QStringList parts;
    QVariantList uniqueAuthors;
    for (int i = sourceRow; i >= 0; i--) {
        parts += sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::GenericDisplayRole).toString();
        QVariant nextAuthor = sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorRole);
        if (!uniqueAuthors.contains(nextAuthor)) {
            uniqueAuthors.append(nextAuthor);
        }
        if (sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::DelegateTypeRole)
                != MessageEventModel::DelegateType::State // If it's not a state event
            || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
        ) {
            break;
        }
    }
    parts.sort(); // Sort them so that all identical events can be collected.
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
            if (count > 1 && uniqueAuthors.length() == 1) {
                chunks.last() += i18ncp("n times", " %1 time ", " %1 times ", count);
            }
        }
        chunks.removeDuplicates();
        QString text = "<style>a {text-decoration: none;}</style>"; // There can be links in the event text so make sure all are styled.
        // The author text is either "n users" if > 1 user or the matrix.to link to a single user.
        QString userText = uniqueAuthors.length() > 1 ? i18ncp("n users", " %1 user ", " %1 users ", uniqueAuthors.length())
                                                      : QStringLiteral("<a href=\"https://matrix.to/#/%1\" style=\"color: %2\">%3</a> ")
                                                            .arg(uniqueAuthors[0].toMap()["id"].toString(),
                                                                 uniqueAuthors[0].toMap()["color"].toString(),
                                                                 uniqueAuthors[0].toMap()["displayName"].toString());
        text += userText;
        text += chunks.takeFirst();

        if (chunks.size() > 0) {
            while (chunks.size() > 1) {
                text += i18nc("[action 1], [action 2 and/or action 3]", ", ");
                text += chunks.takeFirst();
            }
            text += uniqueAuthors.length() > 1 ? i18nc("[action 1, action 2] or [action 3]", " or ") : i18nc("[action 1, action 2] and [action 3]", " and ");
            text += chunks.takeFirst();
        }
        return text;
    } else {
        return {};
    }
}

QVariantList CollapseStateProxyModel::stateEventsList(int sourceRow) const
{
    QVariantList stateEvents;
    for (int i = sourceRow; i >= 0; i--) {
        auto nextState = QVariantMap{
            {"author", sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorRole)},
            {"authorDisplayName", sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorDisplayNameRole).toString()},
            {"text", sourceModel()->data(sourceModel()->index(i, 0), Qt::DisplayRole).toString()},
        };
        stateEvents.append(nextState);
        if (sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::DelegateTypeRole)
                != MessageEventModel::DelegateType::State // If it's not a state event
            || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
        ) {
            break;
        }
    }
    return stateEvents;
}

QVariantList CollapseStateProxyModel::authorList(int sourceRow) const
{
    QVariantList uniqueAuthors;
    for (int i = sourceRow; i >= 0; i--) {
        QVariant nextAvatar = sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorRole);
        if (!uniqueAuthors.contains(nextAvatar)) {
            uniqueAuthors.append(nextAvatar);
        }
        if (sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::DelegateTypeRole)
                != MessageEventModel::DelegateType::State // If it's not a state event
            || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
        ) {
            break;
        }
    }

    if (uniqueAuthors.count() > 5) {
        uniqueAuthors = uniqueAuthors.mid(0, 5);
    }
    return uniqueAuthors;
}

QString CollapseStateProxyModel::excessAuthors(int row) const
{
    QVariantList uniqueAuthors;
    for (int i = row; i >= 0; i--) {
        QVariant nextAvatar = sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorRole);
        if (!uniqueAuthors.contains(nextAvatar)) {
            uniqueAuthors.append(nextAvatar);
        }
        if (sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::DelegateTypeRole)
                != MessageEventModel::DelegateType::State // If it's not a state event
            || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
        ) {
            break;
        }
    }

    int excessAuthors;
    if (uniqueAuthors.count() > 5) {
        excessAuthors = uniqueAuthors.count() - 5;
    } else {
        excessAuthors = 0;
    }
    QString excessAuthorsString;
    if (excessAuthors == 0) {
        return QString();
    } else {
        return QStringLiteral("+ %1").arg(excessAuthors);
    }
}
