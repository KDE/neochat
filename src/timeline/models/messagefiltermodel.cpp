// SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "messagefiltermodel.h"

#include <KLocalizedString>
#include <QVariant>

#include "enums/delegatetype.h"
#include "messagemodel.h"
#include "models/timelinemodel.h"

using namespace Quotient;

bool MessageFilterModel::m_showAllEvents = false;
bool MessageFilterModel::m_showDeletedMessages = false;

MessageFilterModel::MessageFilterModel(QObject *parent, QAbstractItemModel *sourceModel)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(sourceModel);
    setSourceModel(sourceModel);

    if (auto model = dynamic_cast<TimelineModel *>(sourceModel)) {
        connect(model->timelineMessageModel(), &MessageModel::readMarkerIndexChanged, this, &MessageFilterModel::readMarkerIndexChanged);
    }
}

QPersistentModelIndex MessageFilterModel::readMarkerIndex() const
{
    // Check if sourceModel is a message model.
    auto messageModel = dynamic_cast<MessageModel *>(sourceModel());
    bool timelineModelIsSource = false;
    // See if it's a timeline model.
    if (!messageModel) {
        if (const auto timelineModel = dynamic_cast<TimelineModel *>(sourceModel())) {
            messageModel = timelineModel->timelineMessageModel();
            timelineModelIsSource = true;
            if (!messageModel) {
                return {};
            }
        }
    }

    auto eventIndex = messageModel->readMarkerIndex();
    if (!eventIndex.isValid()) {
        return {};
    }

    if (timelineModelIsSource) {
        eventIndex = dynamic_cast<TimelineModel *>(sourceModel())->mapFromSource(eventIndex);
    }
    return mapFromSource(eventIndex);
}

bool MessageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_showAllEvents) {
        return true;
    }
    return eventIsVisible(sourceRow, sourceParent);
}

bool MessageFilterModel::eventIsVisible(int sourceRow, const QModelIndex &sourceParent) const
{
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    // Don't show redacted (i.e. deleted) messages.
    if (index.data(TimelineMessageModel::IsRedactedRole).toBool() && !m_showDeletedMessages) {
        return false;
    }

    // Don't show hidden or replaced messages.
    const int specialMarks = index.data(TimelineMessageModel::SpecialMarksRole).toInt();
    if (specialMarks == EventStatus::Hidden || specialMarks == EventStatus::Replaced) {
        return false;
    }

    // Don't show events with an unknown type.
    const auto eventType = index.data(TimelineMessageModel::DelegateTypeRole).toInt();
    if (eventType == DelegateType::Other) {
        return false;
    }

    // Don't show state events that are not the first in a consecutive group on the
    // same day as they will be grouped as a single delegate.
    const bool notLastRow = sourceRow < sourceModel()->rowCount() - 1;
    const bool previousEventIsState =
        notLastRow ? sourceModel()->data(sourceModel()->index(sourceRow + 1, 0), TimelineMessageModel::DelegateTypeRole) == DelegateType::State : false;
    const bool newDay = sourceModel()->data(sourceModel()->index(sourceRow, 0), TimelineMessageModel::ShowSectionRole).toBool();
    if (eventType == DelegateType::State && notLastRow && previousEventIsState && !newDay) {
        return false;
    }

    return true;
}

QVariant MessageFilterModel::data(const QModelIndex &index, int role) const
{
    if (role == TimelineMessageModel::DelegateTypeRole && m_showAllEvents) {
        if (!eventIsVisible(index.row(), index.parent())) {
            return DelegateType::Other;
        }
    } else if (role == AggregateDisplayRole) {
        return aggregateEventToString(mapToSource(index).row());
    } else if (role == StateEventsRole) {
        return stateEventsList(mapToSource(index).row());
    } else if (role == AuthorListRole) {
        return authorList(mapToSource(index).row());
    } else if (role == ExcessAuthorsRole) {
        return excessAuthors(mapToSource(index).row());
    } else if (role == MessageModel::ShowAuthorRole) {
        return showAuthor(index);
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
    return roles;
}

QModelIndex MessageFilterModel::indexForEventId(const QString &eventId) const
{
    // Check if sourceModel is a message model.
    auto messageModel = dynamic_cast<MessageModel *>(sourceModel());
    bool timelineModelIsSource = false;
    // See if it's a timeline model.
    if (!messageModel) {
        if (const auto timelineModel = dynamic_cast<TimelineModel *>(sourceModel())) {
            messageModel = timelineModel->timelineMessageModel();
            timelineModelIsSource = true;
            if (!messageModel) {
                return {};
            }
        }
    }

    auto eventIndex = messageModel->indexForEventId(eventId);
    if (!eventIndex.isValid()) {
        return {};
    }

    if (timelineModelIsSource) {
        eventIndex = dynamic_cast<TimelineModel *>(sourceModel())->mapFromSource(eventIndex);
    }
    return mapFromSource(eventIndex);
}

const Quotient::RoomEvent *MessageFilterModel::findEvent(const QString &eventId) const
{
    // Check if sourceModel is a message model.
    auto messageModel = dynamic_cast<MessageModel *>(sourceModel());
    // See if it's a timeline model.
    if (!messageModel) {
        if (const auto timelineModel = dynamic_cast<TimelineModel *>(sourceModel())) {
            messageModel = timelineModel->timelineMessageModel();
            if (!messageModel) {
                return nullptr;
            }
        }
    }

    return messageModel->findEvent(eventId);
}

bool MessageFilterModel::showAuthor(QModelIndex index) const
{
    for (auto r = index.row() + 1; r < rowCount(); ++r) {
        auto i = this->index(r, 0);
        // Note !itemData(i).empty() is a check for instances where rows have been removed, e.g. when the read marker is moved.
        // While the row is removed the subsequent row indexes are not changed so we need to skip over the removed index.
        // See - https://doc.qt.io/qt-5/qabstractitemmodel.html#beginRemoveRows
        if (data(i, TimelineMessageModel::SpecialMarksRole) != EventStatus::Hidden && !itemData(i).empty()) {
            return data(i, TimelineMessageModel::AuthorRole) != data(index, TimelineMessageModel::AuthorRole)
                || data(i, TimelineMessageModel::DelegateTypeRole) == DelegateType::State
                || data(i, TimelineMessageModel::TimeRole).toDateTime().msecsTo(data(index, TimelineMessageModel::TimeRole).toDateTime()) > 600000
                || data(i, TimelineMessageModel::TimeRole).toDateTime().toLocalTime().date().day()
                != data(index, TimelineMessageModel::TimeRole).toDateTime().toLocalTime().date().day();
        }
    }

    return true;
}

QString MessageFilterModel::aggregateEventToString(int sourceRow) const
{
    QString aggregateString;
    for (int i = sourceRow; i >= 0; i--) {
        aggregateString += sourceModel()->data(sourceModel()->index(i, 0), TimelineMessageModel::GenericDisplayRole).toString();
        aggregateString += ", "_L1;
        QVariant nextAuthor = sourceModel()->data(sourceModel()->index(i, 0), TimelineMessageModel::AuthorRole);
        if (i > 0
            && (sourceModel()->data(sourceModel()->index(i - 1, 0), TimelineMessageModel::DelegateTypeRole) != DelegateType::State // If it's not a state event
                || sourceModel()->data(sourceModel()->index(i - 1, 0), TimelineMessageModel::ShowSectionRole).toBool() // or the section needs to be visible
                )) {
            break;
        }
    }

    aggregateString = aggregateString.trimmed();
    if (aggregateString.endsWith(u',')) {
        aggregateString.removeLast();
    }
    return aggregateString;
}

QVariantList MessageFilterModel::stateEventsList(int sourceRow) const
{
    QVariantList stateEvents;
    for (int i = sourceRow; i >= 0; i--) {
        auto nextState = QVariantMap{
            {u"eventId"_s, sourceModel()->data(sourceModel()->index(i, 0), TimelineMessageModel::EventIdRole)},
            {u"author"_s, sourceModel()->data(sourceModel()->index(i, 0), TimelineMessageModel::AuthorRole)},
            {u"authorDisplayName"_s, sourceModel()->data(sourceModel()->index(i, 0), TimelineMessageModel::AuthorDisplayNameRole).toString()},
            {u"text"_s, sourceModel()->data(sourceModel()->index(i, 0), Qt::DisplayRole).toString()},
        };
        stateEvents.append(nextState);
        if (i > 0
            && (sourceModel()->data(sourceModel()->index(i - 1, 0), TimelineMessageModel::DelegateTypeRole) != DelegateType::State // If it's not a state event
                || sourceModel()->data(sourceModel()->index(i - 1, 0), TimelineMessageModel::ShowSectionRole).toBool() // or the section needs to be visible
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
        QVariant nextAvatar = sourceModel()->data(sourceModel()->index(i, 0), TimelineMessageModel::AuthorRole);
        if (!uniqueAuthors.contains(nextAvatar)) {
            uniqueAuthors.append(nextAvatar);
        }
        if (i > 0
            && (sourceModel()->data(sourceModel()->index(i - 1, 0), TimelineMessageModel::DelegateTypeRole) != DelegateType::State // If it's not a state event
                || sourceModel()->data(sourceModel()->index(i - 1, 0), TimelineMessageModel::ShowSectionRole).toBool() // or the section needs to be visible
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
        QVariant nextAvatar = sourceModel()->data(sourceModel()->index(i, 0), TimelineMessageModel::AuthorRole);
        if (!uniqueAuthors.contains(nextAvatar)) {
            uniqueAuthors.append(nextAvatar);
        }
        if (i > 0
            && (sourceModel()->data(sourceModel()->index(i - 1, 0), TimelineMessageModel::DelegateTypeRole) != DelegateType::State // If it's not a state event
                || sourceModel()->data(sourceModel()->index(i - 1, 0), TimelineMessageModel::ShowSectionRole).toBool() // or the section needs to be visible
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
        return u"+ %1"_s.arg(excessAuthors);
    }
}

void MessageFilterModel::setShowAllEvents(bool enabled)
{
    MessageFilterModel::m_showAllEvents = enabled;
}

void MessageFilterModel::setShowDeletedMessages(bool enabled)
{
    MessageFilterModel::m_showDeletedMessages = enabled;
}

#include "moc_messagefiltermodel.cpp"
