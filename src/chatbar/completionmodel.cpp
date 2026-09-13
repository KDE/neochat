// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "completionmodel.h"

#include <QPointer>
#include <QTextCursor>

#include "chatbarlogging.h"
#include "completionlist.h"

CompletionModel::CompletionModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QList<CompletionList *> CompletionModel::completionLists() const
{
    QList<CompletionList *> lists;
    std::ranges::for_each(m_lists, [&lists](const QPointer<CompletionList> &list) {
        lists.append(list.get());
    });
    return lists;
}

void CompletionModel::setCompletionLists(const QList<CompletionList *> &completionLists)
{
    if (completionLists == this->completionLists()) {
        return;
    }

    std::ranges::for_each(m_lists, [this](const QPointer<CompletionList> &list) {
        list->disconnect(this);
    });

    beginResetModel();
    m_lists.clear();
    std::ranges::for_each(completionLists, [this](CompletionList *list) {
        m_lists.append(list);
        connect(list, &CompletionList::completionsAdded, this, &CompletionModel::listCompletionsAdded);
        connect(list, &CompletionList::completionsRemoved, this, &CompletionModel::listCompletionsRemoved);
    });
    endResetModel();
    Q_EMIT completionListsChanged();
}

std::optional<qsizetype> CompletionModel::baseRowForList(CompletionList *list)
{
    const auto listIt = std::ranges::find_if(m_lists, [list](const QPointer<CompletionList> &mlist) {
        return list == mlist.get();
    });
    if (listIt == m_lists.end()) {
        return std::nullopt;
    }
    qsizetype baseRow = 0;
    auto it = m_lists.begin();
    while (it != listIt) {
        baseRow += (*it)->size();
    }
    return baseRow;
}

void CompletionModel::listCompletionsAdded(CompletionList *list, qsizetype first, qsizetype last)
{
    const auto baseRow = baseRowForList(list);
    if (!baseRow) {
        return;
    }
    beginInsertRows({}, *baseRow + first, *baseRow + last);
    endInsertRows();
}

void CompletionModel::listCompletionsRemoved(CompletionList *list, qsizetype first, qsizetype last)
{
    const auto baseRow = baseRowForList(list);
    if (!baseRow) {
        return;
    }
    beginRemoveRows({}, *baseRow + first, *baseRow + last);
    endInsertRows();
}

std::optional<Completion> CompletionModel::completionAtRow(qsizetype row) const
{
    auto localRow = row;
    const auto it = std::ranges::find_if(m_lists, [&localRow](CompletionList *list) {
        if (localRow < list->size()) {
            return true;
        }
        localRow -= list->size();
        return false;
    });
    if (it == m_lists.end()) {
        return std::nullopt;
    }
    return (*it)->at(localRow);
}

QVariant CompletionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        qCWarning(ChatBar) << __FUNCTION__ << "called with invalid index" << index << rowCount();
        return {};
    }

    const auto completion = completionAtRow(index.row());
    if (!completion) {
        return {};
    }

    if (role == TitleRole) {
        return completion->title;
    }
    if (role == SubtitleRole) {
        return completion->description;
    }
    if (role == AvatarSourceRole) {
        return completion->avatarSource;
    }
    if (role == StartSequenceRole) {
        return completion->startsequence;
    }
    if (role == MatchSequencesRole) {
        return completion->matchSequences;
    }
    if (role == ReplaceStringRole) {
        return completion->replaceString;
    }
    if (role == HRefRole) {
        return completion->hRef;
    }

    return {};
}

int CompletionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    qsizetype rows = 0;
    std::ranges::for_each(m_lists, [&rows](CompletionList *list) {
        rows += list->size();
    });
    return rows;
}

QHash<int, QByteArray> CompletionModel::roleNames() const
{
    return {
        {TitleRole, "title"},
        {SubtitleRole, "subtitle"},
        {AvatarSourceRole, "avatarSource"},
        {ReplaceStringRole, "replaceString"},
        {HRefRole, "hRef"},
    };
}

#include "moc_completionmodel.cpp"
