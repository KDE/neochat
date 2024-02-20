// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "messagefiltermodel.h"

#include <KLocalizedString>

#include "enums/delegatetype.h"
#include "messageeventmodel.h"
#include "neochatconfig.h"
#include "timelinemodel.h"

using namespace Quotient;

MessageFilterModel::MessageFilterModel(QObject *parent, TimelineModel *sourceModel)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(sourceModel);
    setSourceModel(sourceModel);

    connect(NeoChatConfig::self(), &NeoChatConfig::ShowStateEventChanged, this, [this] {
        invalidateFilter();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowLeaveJoinEventChanged, this, [this] {
        invalidateFilter();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowRenameChanged, this, [this] {
        invalidateFilter();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowAvatarUpdateChanged, this, [this] {
        invalidateFilter();
    });
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowDeletedMessagesChanged, this, [this] {
        invalidateFilter();
    });
}

bool MessageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    // Don't show redacted (i.e. deleted) messages.
    if (index.data(MessageEventModel::IsRedactedRole).toBool() && !NeoChatConfig::self()->showDeletedMessages()) {
        return false;
    }

    // Don't show hidden or replaced messages.
    const int specialMarks = index.data(MessageEventModel::SpecialMarksRole).toInt();
    if (specialMarks == EventStatus::Hidden || specialMarks == EventStatus::Replaced) {
        return false;
    }

    // Don't show events with an unknown type.
    const auto eventType = index.data(MessageEventModel::DelegateTypeRole).toInt();
    if (eventType == DelegateType::Other) {
        return false;
    }

    // Don't show state events that are not the first in a consecutive group on the
    // same day as they will be grouped as a single delegate.
    const bool notLastRow = sourceRow < sourceModel()->rowCount() - 1;
    const bool previousEventIsState = notLastRow
        ? sourceModel()->data(sourceModel()->index(sourceRow + 1, 0), MessageEventModel::DelegateTypeRole) == DelegateType::State
        : false;
    const bool newDay = sourceModel()->data(sourceModel()->index(sourceRow, 0), MessageEventModel::ShowSectionRole).toBool();
    if (eventType == DelegateType::State && notLastRow && previousEventIsState && !newDay) {
        return false;
    }

    return true;
}

QVariant MessageFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == AggregateDisplayRole) {
        return aggregateEventToString(mapToSource(index).row());
    } else if (role == StateEventsRole) {
        return stateEventsList(mapToSource(index).row());
    } else if (role == AuthorListRole) {
        return authorList(mapToSource(index).row());
    } else if (role == ExcessAuthorsRole) {
        return excessAuthors(mapToSource(index).row());
    } else if (role == ShowAuthorRole) {
        for (auto r = index.row() + 1; r < rowCount(); ++r) {
            auto i = this->index(r, 0);
            // Note !itemData(i).empty() is a check for instances where rows have been removed, e.g. when the read marker is moved.
            // While the row is removed the subsequent row indexes are not changed so we need to skip over the removed index.
            // See - https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
            if (data(i, MessageEventModel::SpecialMarksRole) != EventStatus::Hidden && !itemData(i).empty()) {
                return data(i, MessageEventModel::AuthorRole) != data(index, MessageEventModel::AuthorRole)
                    || data(i, MessageEventModel::DelegateTypeRole) == DelegateType::State
                    || data(i, MessageEventModel::TimeRole).toDateTime().msecsTo(data(index, MessageEventModel::TimeRole).toDateTime()) > 600000
                    || data(i, MessageEventModel::TimeRole).toDateTime().toLocalTime().date().day()
                    != data(index, MessageEventModel::TimeRole).toDateTime().toLocalTime().date().day();
            }
        }

        return true;
    }
    return QSortFilterProxyModel::data(index, role);
}

QHash<int, QByteArray> MessageFilterModel::roleNames() const
{
    auto roles = sourceModel() ? sourceModel()->roleNames() : QHash<int, QByteArray>();
    roles[AggregateDisplayRole] = "aggregateDisplay";
    roles[StateEventsRole] = "stateEvents";
    roles[AuthorListRole] = "authorList";
    roles[ExcessAuthorsRole] = "excessAuthors";
    roles[ShowAuthorRole] = "showAuthor";
    return roles;
}

QString MessageFilterModel::aggregateEventToString(int sourceRow) const
{
    QStringList parts;
    QVariantList uniqueAuthors;
    for (int i = sourceRow; i >= 0; i--) {
        parts += sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::GenericDisplayRole).toString();
        QVariant nextAuthor = sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorRole);
        if (!uniqueAuthors.contains(nextAuthor)) {
            uniqueAuthors.append(nextAuthor);
        }
        if (i > 0
            && (sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::DelegateTypeRole) != DelegateType::State // If it's not a state event
                || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
                )) {
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
            while (!parts.isEmpty() && parts.first() == part) {
                parts.removeFirst();
                count++;
            }
            chunks.last() += i18ncp("%1: What's being done; %2: How often it is done.", " %1", " %1 %2 times", part, count);
        }
        chunks.removeDuplicates();
        // The author text is either "n users" if > 1 user or the matrix.to link to a single user.
        QString userText = uniqueAuthors.length() > 1 ? i18ncp("n users", " %1 user ", " %1 users ", uniqueAuthors.length())
                                                      : QStringLiteral("<a href=\"https://matrix.to/#/%1\">%3</a> ")
                                                            .arg(uniqueAuthors[0].toMap()[QStringLiteral("id")].toString(),
                                                                 uniqueAuthors[0].toMap()[QStringLiteral("displayName")].toString().toHtmlEscaped());

        QString chunksText;
        chunksText += chunks.takeFirst();
        if (chunks.size() > 0) {
            while (chunks.size() > 1) {
                chunksText += i18nc("[action 1], [action 2 and/or action 3]", ", ");
                chunksText += chunks.takeFirst();
            }
            chunksText +=
                uniqueAuthors.length() > 1 ? i18nc("[action 1, action 2] or [action 3]", " or ") : i18nc("[action 1, action 2] and [action 3]", " and ");
            chunksText += chunks.takeFirst();
        }
        return i18nc(
            "userText (%1) is either a Matrix username if a single user sent all the states or n users if they were sent by multiple users."
            "chunksText (%2) is a list of comma separated actions for each of the state events in the group.",
            "<style>a {text-decoration: none;}</style>%1 %2",
            userText,
            chunksText);
    } else {
        return {};
    }
}

QVariantList MessageFilterModel::stateEventsList(int sourceRow) const
{
    QVariantList stateEvents;
    for (int i = sourceRow; i >= 0; i--) {
        auto nextState = QVariantMap{
            {QStringLiteral("author"), sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorRole)},
            {QStringLiteral("authorDisplayName"), sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorDisplayNameRole).toString()},
            {QStringLiteral("text"), sourceModel()->data(sourceModel()->index(i, 0), Qt::DisplayRole).toString()},
        };
        stateEvents.append(nextState);
        if (i > 0
            && (sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::DelegateTypeRole) != DelegateType::State // If it's not a state event
                || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
                )) {
            break;
        }
    }
    return stateEvents;
}

QVariantList MessageFilterModel::authorList(int sourceRow) const
{
    QVariantList uniqueAuthors;
    for (int i = sourceRow; i >= 0; i--) {
        QVariant nextAvatar = sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorRole);
        if (!uniqueAuthors.contains(nextAvatar)) {
            uniqueAuthors.append(nextAvatar);
        }
        if (i > 0
            && (sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::DelegateTypeRole) != DelegateType::State // If it's not a state event
                || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
                )) {
            break;
        }
    }

    if (uniqueAuthors.count() > 5) {
        uniqueAuthors = uniqueAuthors.mid(0, 5);
    }
    return uniqueAuthors;
}

QString MessageFilterModel::excessAuthors(int row) const
{
    QVariantList uniqueAuthors;
    for (int i = row; i >= 0; i--) {
        QVariant nextAvatar = sourceModel()->data(sourceModel()->index(i, 0), MessageEventModel::AuthorRole);
        if (!uniqueAuthors.contains(nextAvatar)) {
            uniqueAuthors.append(nextAvatar);
        }
        if (i > 0
            && (sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::DelegateTypeRole) != DelegateType::State // If it's not a state event
                || sourceModel()->data(sourceModel()->index(i - 1, 0), MessageEventModel::ShowSectionRole).toBool() // or the section needs to be visible
                )) {
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

#include "moc_messagefiltermodel.cpp"
