// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "completionmodel.h"
#include <QDebug>

#include "actionsmodel.h"
#include "completionproxymodel.h"
#include "customemojimodel.h"
#include "emojimodel.h"
#include "neochatroom.h"
#include "userlistmodel.h"

CompletionModel::CompletionModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_filterModel(new CompletionProxyModel())
    , m_userListModel(new UserListModel(this))
    , m_emojiModel(new QConcatenateTablesProxyModel(this))
{
    connect(this, &CompletionModel::textChanged, this, &CompletionModel::updateCompletion);
    connect(this, &CompletionModel::roomChanged, this, [this]() {
        m_userListModel->setRoom(m_room);
    });
    m_emojiModel->addSourceModel(&CustomEmojiModel::instance());
    m_emojiModel->addSourceModel(&EmojiModel::instance());
}

QString CompletionModel::text() const
{
    return m_text;
}

void CompletionModel::setText(const QString &text, const QString &fullText)
{
    m_text = text;
    m_fullText = fullText;
    Q_EMIT textChanged();
}

int CompletionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_autoCompletionType == None) {
        return 0;
    }
    return m_filterModel->rowCount();
}

QVariant CompletionModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_filterModel->rowCount()) {
        return {};
    }
    auto filterIndex = m_filterModel->index(index.row(), 0);
    if (m_autoCompletionType == User) {
        if (role == DisplayNameRole) {
            return m_filterModel->data(filterIndex, UserListModel::DisplayNameRole);
        }
        if (role == SubtitleRole) {
            return m_filterModel->data(filterIndex, UserListModel::UserIdRole);
        }
        if (role == IconNameRole) {
            return m_filterModel->data(filterIndex, UserListModel::AvatarRole);
        }
    }

    if (m_autoCompletionType == Command) {
        if (role == DisplayNameRole) {
            return QStringLiteral("%1 %2").arg(m_filterModel->data(filterIndex, ActionsModel::Prefix).toString(),
                                               m_filterModel->data(filterIndex, ActionsModel::Parameters).toString());
        }
        if (role == SubtitleRole) {
            return m_filterModel->data(filterIndex, ActionsModel::Description);
        }
        if (role == IconNameRole) {
            return QStringLiteral("invalid");
        }
        if (role == ReplacedTextRole) {
            return m_filterModel->data(filterIndex, ActionsModel::Prefix);
        }
    }
    if (m_autoCompletionType == Room) {
        if (role == DisplayNameRole) {
            return m_filterModel->data(filterIndex, RoomListModel::DisplayNameRole);
        }
        if (role == SubtitleRole) {
            return m_filterModel->data(filterIndex, RoomListModel::CanonicalAliasRole);
        }
        if (role == IconNameRole) {
            auto mediaId = m_filterModel->data(filterIndex, RoomListModel::AvatarRole).toString();
            if (mediaId.isEmpty()) {
                return QVariant();
            }
            if (m_room) {
                return m_room->connection()->makeMediaUrl(QUrl(QStringLiteral("mxc://%1").arg(mediaId)));
            }
        }
    }
    if (m_autoCompletionType == Emoji) {
        if (role == DisplayNameRole) {
            return m_filterModel->data(filterIndex, CustomEmojiModel::DisplayRole);
        }
        if (role == IconNameRole) {
            return m_filterModel->data(filterIndex, CustomEmojiModel::MxcUrl);
        }
        if (role == ReplacedTextRole) {
            return m_filterModel->data(filterIndex, CustomEmojiModel::ReplacedTextRole);
        }
        if (role == SubtitleRole) {
            return m_filterModel->data(filterIndex, EmojiModel::DescriptionRole);
        }
    }

    return {};
}

QHash<int, QByteArray> CompletionModel::roleNames() const
{
    return {
        {DisplayNameRole, "displayName"},
        {SubtitleRole, "subtitle"},
        {IconNameRole, "iconName"},
        {ReplacedTextRole, "replacedText"},
    };
}

void CompletionModel::updateCompletion()
{
    if (text().startsWith(QLatin1Char('@'))) {
        m_filterModel->setSourceModel(m_userListModel);
        m_filterModel->setFilterRole(UserListModel::UserIdRole);
        m_filterModel->setSecondaryFilterRole(UserListModel::DisplayNameRole);
        m_filterModel->setFullText(m_fullText);
        m_filterModel->setFilterText(m_text);
        m_autoCompletionType = User;
        m_filterModel->invalidate();
    } else if (text().startsWith(QLatin1Char('/'))) {
        m_filterModel->setSourceModel(&ActionsModel::instance());
        m_filterModel->setFilterRole(ActionsModel::Prefix);
        m_filterModel->setSecondaryFilterRole(-1);
        m_filterModel->setFullText(m_fullText);
        m_filterModel->setFilterText(m_text.mid(1));
        m_autoCompletionType = Command;
        m_filterModel->invalidate();
    } else if (text().startsWith(QLatin1Char('#'))) {
        m_autoCompletionType = Room;
        m_filterModel->setSourceModel(m_roomListModel);
        m_filterModel->setFilterRole(RoomListModel::CanonicalAliasRole);
        m_filterModel->setSecondaryFilterRole(RoomListModel::DisplayNameRole);
        m_filterModel->setFullText(m_fullText);
        m_filterModel->setFilterText(m_text);
        m_filterModel->invalidate();
    } else if (text().startsWith(QLatin1Char(':')) && text().size() > 1 && !text()[1].isUpper()
               && (m_fullText.indexOf(QLatin1Char(':'), 1) == -1
                   || (m_fullText.indexOf(QLatin1Char(' ')) != -1 && m_fullText.indexOf(QLatin1Char(':'), 1) > m_fullText.indexOf(QLatin1Char(' '), 1)))) {
        m_filterModel->setSourceModel(m_emojiModel);
        m_autoCompletionType = Emoji;
        m_filterModel->setFilterRole(CustomEmojiModel::Name);
        m_filterModel->setSecondaryFilterRole(EmojiModel::DescriptionRole);
        m_filterModel->setFullText(m_fullText);
        m_filterModel->setFilterText(m_text);
        m_filterModel->invalidate();
    } else {
        m_autoCompletionType = None;
    }
    beginResetModel();
    endResetModel();
}

NeoChatRoom *CompletionModel::room() const
{
    return m_room;
}

void CompletionModel::setRoom(NeoChatRoom *room)
{
    m_room = room;
    Q_EMIT roomChanged();
}

CompletionModel::AutoCompletionType CompletionModel::autoCompletionType() const
{
    return m_autoCompletionType;
}

void CompletionModel::setAutoCompletionType(AutoCompletionType autoCompletionType)
{
    m_autoCompletionType = autoCompletionType;
    Q_EMIT autoCompletionTypeChanged();
}

RoomListModel *CompletionModel::roomListModel() const
{
    return m_roomListModel;
}

void CompletionModel::setRoomListModel(RoomListModel *roomListModel)
{
    m_roomListModel = roomListModel;
    Q_EMIT roomListModelChanged();
}

#include "moc_completionmodel.cpp"
