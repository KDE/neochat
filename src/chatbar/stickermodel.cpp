// SPDX-FileCopyrightText: 2021-2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "stickermodel.h"

#include "chatbarlogging.h"

StickerModel::StickerModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QList<StickerList *> StickerModel::stickerLists() const
{
    QList<StickerList *> lists;
    std::ranges::for_each(m_lists, [&lists](const QPointer<StickerList> &list) {
        lists.append(list.get());
    });
    return lists;
}

void StickerModel::setStickerLists(const QList<StickerList *> &stickerLists)
{
    if (stickerLists == this->stickerLists()) {
        return;
    }

    std::ranges::for_each(m_lists, [this](const QPointer<StickerList> &list) {
        list->disconnect(this);
    });

    beginResetModel();
    m_lists.clear();
    std::ranges::for_each(stickerLists, [this](StickerList *list) {
        m_lists.append(list);
        connect(list, &StickerList::stickersAdded, this, &StickerModel::listCompletionsAdded);
        connect(list, &StickerList::stickersRemoved, this, &StickerModel::listCompletionsRemoved);
    });
    endResetModel();
    Q_EMIT stickerListsChanged();
}

void StickerModel::listCompletionsAdded(StickerList *list, qsizetype first, qsizetype last)
{
    if (m_listIndex < 0 || m_listIndex >= m_lists.size() || list != m_lists[m_listIndex]) {
        return;
    }
    beginInsertRows({}, first, last);
    endInsertRows();
}

void StickerModel::listCompletionsRemoved(StickerList *list, qsizetype first, qsizetype last)
{
    if (m_listIndex < 0 || m_listIndex >= m_lists.size() || list != m_lists[m_listIndex]) {
        return;
    }
    beginRemoveRows({}, first, last);
    endInsertRows();
}

int StickerModel::listIndex() const
{
    return m_listIndex;
}

void StickerModel::setListIndex(int listIndex)
{
    if (listIndex == m_listIndex) {
        return;
    }
    m_listIndex = listIndex;
    Q_EMIT listIndexChanged();
}

int StickerModel::rowCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (m_listIndex < 0 || m_listIndex >= m_lists.size()) {
        return 0;
    }
    return m_lists[m_listIndex].get()->size();
}

QVariant StickerModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
        qCWarning(ChatBar) << Q_FUNC_INFO << "called with invalid model index" << index << role;
        return {};
    }
    if (m_listIndex < 0 || m_listIndex >= m_lists.size()) {
        qCWarning(ChatBar) << __FUNCTION__ << "called with invalid list index" << m_listIndex;
        return {};
    }
    return m_lists[m_listIndex]->data(index.row(), role);
}

QHash<int, QByteArray> StickerModel::roleNames() const
{
    return {
        {SourceRole, "source"},
        {NameRole, "name"},
    };
}

#include "moc_stickermodel.cpp"
