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
    qsizetype baseRow = 0;
    for (const QPointer<CompletionList> &listPtr : m_lists) {
        if (listPtr.get() == list) {
            return baseRow;
        } else if (m_currentText.startsWith(listPtr->startSequence())) {
            baseRow += listPtr->size();
        }
    }
    return std::nullopt;
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

QVariant CompletionModel::dataAtRow(qsizetype row, int role) const
{
    auto localRow = row;
    const auto it = std::ranges::find_if(m_lists, [this, &localRow](CompletionList *list) {
        if (!m_currentText.startsWith(list->startSequence())) {
            return false;
        }
        if (localRow < list->size()) {
            return true;
        }
        localRow -= list->size();
        return false;
    });
    if (it == m_lists.end()) {
        return {};
    }
    return (*it)->data(localRow, role);
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

    return dataAtRow(index.row(), role);
}

int CompletionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (m_currentText.isEmpty()) {
        return 0;
    }

    qsizetype rows = 0;
    std::ranges::for_each(m_lists, [this, &rows](CompletionList *list) {
        if (m_currentText.startsWith(list->startSequence())) {
            rows += list->size();
        }
    });
    return rows;
}

QHash<int, QByteArray> CompletionModel::roleNames() const
{
    return {
        {TitleRole, "title"},
        {DescriptionRole, "description"},
        {AvatarSourceRole, "avatarSource"},
        {ReplaceStringRole, "replaceString"},
        {HRefRole, "hRef"},
    };
}

void CompletionModel::setCurrentText(const QString &currentText)
{
    beginResetModel();
    m_currentText = currentText;
    endResetModel();
}

#include "moc_completionmodel.cpp"
