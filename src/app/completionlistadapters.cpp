// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "completionlistadapters.h"

#include "actionsmodel.h"
#include "completionmodel.h"
#include "models/customemojimodel.h"
#include "models/emojimodel.h"
#include "models/roomlistmodel.h"
#include <qcontainerfwd.h>

using namespace Qt::StringLiterals;

UserCompletionList::UserCompletionList(QObject *parent)
    : CompletionList(parent)
{
}

UserListModel *UserCompletionList::userListModel() const
{
    return m_userListModel;
}

void UserCompletionList::setUserListModel(UserListModel *userListModel)
{
    if (userListModel == m_userListModel) {
        return;
    }

    if (m_userListModel) {
        m_userListModel->disconnect(this);
    }

    m_userListModel = userListModel;

    if (m_userListModel) {
        connect(m_userListModel, &UserListModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
            Q_EMIT completionsAdded(this, first, last);
        });
        connect(m_userListModel, &UserListModel::rowsRemoved, this, [this](const QModelIndex &, int first, int last) {
            Q_EMIT completionsRemoved(this, first, last);
        });
    }

    Q_EMIT userListModelChanged();
}

qsizetype UserCompletionList::size() const
{
    if (!m_userListModel) {
        return 0;
    }
    return m_userListModel->rowCount();
}

QVariant UserCompletionList::data(qsizetype row, int role) const
{
    if (!m_userListModel) {
        return {};
    }
    const auto index = m_userListModel->index(row);
    switch (role) {
    case CompletionModel::TitleRole:
    case CompletionModel::ReplaceStringRole:
        return index.data(UserListModel::DisplayNameRole);
    case CompletionModel::DescriptionRole:
        return index.data(UserListModel::UserIdRole);
    case CompletionModel::AvatarSourceRole:
        return index.data(UserListModel::AvatarRole);
    case CompletionModel::StartSequenceRole:
        return startSequence();
    case CompletionModel::MatchSequencesRole: {
        QStringList matchSequences;
        const auto userId = index.data(UserListModel::UserIdRole).toString();
        if (!userId.isEmpty()) {
            matchSequences.append(userId);
        }
        const auto displayName = index.data(UserListModel::DisplayNameRole).toString();
        if (!displayName.isEmpty()) {
            matchSequences.append(displayName);
        }
        return matchSequences;
    }
    case CompletionModel::HRefRole:
        return QUrl(u"https://matrix.to/#/%1"_s.arg(index.data(UserListModel::UserIdRole).toString()));
    default:
        return {};
    }
}

QString UserCompletionList::startSequence() const
{
    return u"@"_s;
}

RoomCompletionList::RoomCompletionList(QObject *parent)
    : CompletionList(parent)
{
}

RoomListModel *RoomCompletionList::roomListModel() const
{
    return m_roomListModel;
}

void RoomCompletionList::setRoomListModel(RoomListModel *roomListModel)
{
    if (roomListModel == m_roomListModel) {
        return;
    }

    if (m_roomListModel) {
        m_roomListModel->disconnect(this);
    }

    m_roomListModel = roomListModel;

    if (m_roomListModel) {
        connect(m_roomListModel, &UserListModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
            Q_EMIT completionsAdded(this, first, last);
        });
        connect(m_roomListModel, &UserListModel::rowsRemoved, this, [this](const QModelIndex &, int first, int last) {
            Q_EMIT completionsRemoved(this, first, last);
        });
    }

    Q_EMIT roomListModelChanged();
}

qsizetype RoomCompletionList::size() const
{
    if (!m_roomListModel) {
        return 0;
    }
    return m_roomListModel->rowCount();
}

QVariant RoomCompletionList::data(qsizetype row, int role) const
{
    if (!m_roomListModel) {
        return {};
    }
    const auto index = m_roomListModel->index(row);
    switch (role) {
    case CompletionModel::TitleRole:
        return index.data(RoomListModel::DisplayNameRole);
    case CompletionModel::DescriptionRole:
        return index.data(RoomListModel::CanonicalAliasRole);
    case CompletionModel::AvatarSourceRole:
        return index.data(RoomListModel::AvatarRole).toUrl();
    case CompletionModel::StartSequenceRole:
        return startSequence();
    case CompletionModel::MatchSequencesRole: {
        const auto canonicalAlias = index.data(RoomListModel::CanonicalAliasRole).toString();
        if (canonicalAlias.isEmpty()) {
            return QStringList();
        }
        QStringList matchSequences = {index.data(RoomListModel::DisplayNameRole).toString()};
        matchSequences += canonicalAlias.last(canonicalAlias.length() - startSequence().length());
        return matchSequences;
    }
    case CompletionModel::ReplaceStringRole:
        return index.data(RoomListModel::CanonicalAliasRole);
    case CompletionModel::HRefRole:
        return QUrl(u"https://matrix.to/#/%1"_s.arg(index.data(RoomListModel::CanonicalAliasRole).toString()));
    default:
        return {};
    }
}

QString RoomCompletionList::startSequence() const
{
    return u"#"_s;
}

ActionsCompletionList::ActionsCompletionList(QObject *parent)
    : CompletionList(parent)
{
    connect(&ActionsModel::instance(), &ActionsModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
        Q_EMIT completionsAdded(this, first, last);
    });
    connect(&ActionsModel::instance(), &ActionsModel::rowsRemoved, this, [this](const QModelIndex &, int first, int last) {
        Q_EMIT completionsRemoved(this, first, last);
    });
}

qsizetype ActionsCompletionList::size() const
{
    return ActionsModel::instance().rowCount();
}

QVariant ActionsCompletionList::data(qsizetype row, int role) const
{
    const auto index = ActionsModel::instance().index(row);
    switch (role) {
    case CompletionModel::TitleRole:
        return u"%1 %2"_s.arg(index.data(ActionsModel::Prefix).toString(), index.data(ActionsModel::Parameters).toString());
    case CompletionModel::DescriptionRole:
        return index.data(ActionsModel::Description).toString();
    case CompletionModel::AvatarSourceRole:
        return {};
    case CompletionModel::StartSequenceRole:
        return startSequence();
    case CompletionModel::MatchSequencesRole:
        return QStringList{index.data(ActionsModel::Prefix).toString()};
    case CompletionModel::ReplaceStringRole:
        return u"/%1"_s.arg(index.data(ActionsModel::Prefix).toString());
    case CompletionModel::HRefRole:
        return {};
    default:
        return {};
    }
}

QString ActionsCompletionList::startSequence() const
{
    return u"/"_s;
}

EmojiCompletionList::EmojiCompletionList(QObject *parent)
    : CompletionList(parent)
{
    connect(&EmojiModel::instance(), &EmojiModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
        Q_EMIT completionsAdded(this, first, last);
    });
    connect(&EmojiModel::instance(), &EmojiModel::rowsRemoved, this, [this](const QModelIndex &, int first, int last) {
        Q_EMIT completionsRemoved(this, first, last);
    });
}

qsizetype EmojiCompletionList::size() const
{
    return EmojiModel::instance().rowCount();
}

QVariant EmojiCompletionList::data(qsizetype row, int role) const
{
    const auto index = EmojiModel::instance().index(row);
    switch (role) {
    case CompletionModel::TitleRole:
        return u"%1 %2"_s.arg(index.data(EmojiModel::UnicodeRole).toString(), index.data(EmojiModel::ShortNameRole).toString());
    case CompletionModel::DescriptionRole:
        return index.data(EmojiModel::DescriptionRole);
    case CompletionModel::AvatarSourceRole:
        return {};
    case CompletionModel::StartSequenceRole:
        return startSequence();
    case CompletionModel::MatchSequencesRole: {
        auto shortNameNoColon = index.data(EmojiModel::ShortNameRole).toString();
        shortNameNoColon.removeFirst();
        shortNameNoColon.removeLast();
        return QStringList{index.data(EmojiModel::UnicodeRole).toString(), shortNameNoColon};
    }
    case CompletionModel::ReplaceStringRole:
        return index.data(EmojiModel::UnicodeRole);
    case CompletionModel::HRefRole:
        return {};
    default:
        return {};
    }
}

QString EmojiCompletionList::startSequence() const
{
    return u":"_s;
}

CustomEmojiCompletionList::CustomEmojiCompletionList(QObject *parent)
    : CompletionList(parent)
{
    connect(&CustomEmojiModel::instance(), &CustomEmojiModel::rowsInserted, this, [this](const QModelIndex &, int first, int last) {
        Q_EMIT completionsAdded(this, first, last);
    });
    connect(&CustomEmojiModel::instance(), &CustomEmojiModel::rowsRemoved, this, [this](const QModelIndex &, int first, int last) {
        Q_EMIT completionsRemoved(this, first, last);
    });
}

qsizetype CustomEmojiCompletionList::size() const
{
    return CustomEmojiModel::instance().rowCount();
}

QVariant CustomEmojiCompletionList::data(qsizetype row, int role) const
{
    const auto index = CustomEmojiModel::instance().index(row);
    switch (role) {
    case CompletionModel::TitleRole:
        return index.data(CustomEmojiModel::Name);
    case CompletionModel::DescriptionRole:
        return index.data(CustomEmojiModel::DescriptionRole);
    case CompletionModel::AvatarSourceRole:
        return index.data(CustomEmojiModel::MxcUrl);
    case CompletionModel::StartSequenceRole:
        return startSequence();
    case CompletionModel::MatchSequencesRole: {
        auto nameNoColon = index.data(CustomEmojiModel::Name).toString();
        nameNoColon.removeFirst();
        nameNoColon.removeLast();
        return QStringList{nameNoColon};
    }
    case CompletionModel::ReplaceStringRole:
        return index.data(CustomEmojiModel::Name);
    case CompletionModel::HRefRole:
        return {};
    default:
        return {};
    }
}

QString CustomEmojiCompletionList::startSequence() const
{
    return u":"_s;
}

#include "moc_completionlistadapters.cpp"
