// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "completionlistadapters.h"
#include "actionsmodel.h"
#include "models/customemojimodel.h"
#include "models/emojimodel.h"

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

std::optional<Completion> UserCompletionList::at(qsizetype i) const
{
    if (!m_userListModel) {
        return std::nullopt;
    }
    const auto index = m_userListModel->index(i);
    const auto displayName = index.data(UserListModel::DisplayNameRole).toString();
    const auto userId = index.data(UserListModel::UserIdRole).toString();
    return Completion{
        .title = displayName,
        .description = userId,
        .avatarSource = index.data(UserListModel::AvatarRole).toUrl(),
        .startsequence = u"@"_s,
        .matchSequences = {displayName, userId.last(userId.size() - 1)},
        .replaceString = displayName,
        .hRef = QUrl(u"https://matrix.to/#/%1"_s.arg(userId)),
    };
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

std::optional<Completion> RoomCompletionList::at(qsizetype i) const
{
    if (!m_roomListModel) {
        return std::nullopt;
    }
    const auto index = m_roomListModel->index(i);
    const auto displayName = index.data(RoomListModel::DisplayNameRole).toString();
    const auto canonicalAlias = index.data(RoomListModel::CanonicalAliasRole).toString();
    QStringList matchSequences = {displayName};
    if (!canonicalAlias.isEmpty()) {
        matchSequences += canonicalAlias.last(canonicalAlias.size() - 1);
    }
    return Completion{
        .title = displayName,
        .description = canonicalAlias,
        .avatarSource = index.data(RoomListModel::AvatarRole).toUrl(),
        .startsequence = u"#"_s,
        .matchSequences = matchSequences,
        .replaceString = canonicalAlias,
        .hRef = QUrl(u"https://matrix.to/#/%1"_s.arg(canonicalAlias)),
    };
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

std::optional<Completion> ActionsCompletionList::at(qsizetype i) const
{
    const auto index = ActionsModel::instance().index(i);
    const auto prefix = index.data(ActionsModel::Prefix).toString();
    return Completion{
        .title = u"%1 %2"_s.arg(prefix, index.data(ActionsModel::Parameters).toString()),
        .description = index.data(ActionsModel::Description).toString(),
        .avatarSource = {},
        .startsequence = u"/"_s,
        .matchSequences = {prefix},
        .replaceString = u"/%1"_s.arg(prefix),
        .hRef = {},
    };
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

std::optional<Completion> EmojiCompletionList::at(qsizetype i) const
{
    const auto index = EmojiModel::instance().index(i);
    const auto unicode = index.data(EmojiModel::UnicodeRole).toString();
    const auto shortName = index.data(EmojiModel::ShortNameRole).toString();
    auto shortNameNoColon = shortName;
    shortNameNoColon.removeFirst();
    shortNameNoColon.removeLast();
    return Completion{
        .title = u"%1 %2"_s.arg(unicode, shortName),
        .description = index.data(EmojiModel::DescriptionRole).toString(),
        .avatarSource = {},
        .startsequence = u":"_s,
        .matchSequences = {unicode, shortNameNoColon},
        .replaceString = unicode,
        .hRef = {},
    };
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

std::optional<Completion> CustomEmojiCompletionList::at(qsizetype i) const
{
    const auto index = CustomEmojiModel::instance().index(i);
    const auto name = index.data(CustomEmojiModel::Name).toString();
    auto nameNoColon = name;
    nameNoColon.removeFirst();
    nameNoColon.removeLast();
    const auto url = index.data(CustomEmojiModel::MxcUrl).toUrl();
    return Completion{
        .title = name,
        .description = index.data(EmojiModel::DescriptionRole).toString(),
        .avatarSource = url,
        .startsequence = u":"_s,
        .matchSequences = {nameNoColon},
        .replaceString = name,
        .hRef = {},
    };
}

#include "moc_completionlistadapters.cpp"
