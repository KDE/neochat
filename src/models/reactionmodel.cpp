// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "reactionmodel.h"

#include <QDebug>

#include <KLocalizedString>

#include <Quotient/user.h>

ReactionModel::ReactionModel(QObject *parent, QList<Reaction> reactions, Quotient::User *localUser)
    : QAbstractListModel(parent)
    , m_localUser(localUser)
{
    setReactions(reactions);
}

QVariant ReactionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() >= rowCount()) {
        qDebug() << "ReactionModel, something's wrong: index.row() >= rowCount()";
        return {};
    }

    const auto &reaction = m_reactions.at(index.row());

    if (role == TextRole) {
        if (reaction.authors.count() > 1) {
            return QStringLiteral("%1  %2").arg(reaction.reaction, QString::number(reaction.authors.count()));
        } else {
            return reaction.reaction;
        }
    }

    if (role == ReactionRole) {
        return reaction.reaction;
    }

    if (role == ToolTipRole) {
        QString text;

        for (int i = 0; i < reaction.authors.count() && i < 3; i++) {
            if (i != 0) {
                if (i < reaction.authors.count() - 1) {
                    text += QStringLiteral(", ");
                } else {
                    text += i18nc("Separate the usernames of users", " and ");
                }
            }
            text += reaction.authors.at(i).toMap()[QStringLiteral("displayName")].toString();
        }

        if (reaction.authors.count() > 3) {
            text += i18ncp("%1 is the number of other users", " and %1 other", " and %1 others", reaction.authors.count() - 3);
        }

        text = i18ncp("%2 is the users who reacted and %3 the emoji that was given",
                      "%2 reacted with %3",
                      "%2 reacted with %3",
                      reaction.authors.count(),
                      text,
                      reaction.reaction);
        return text;
    }

    if (role == AuthorsRole) {
        return reaction.authors;
    }

    if (role == HasLocalUser) {
        for (auto author : reaction.authors) {
            if (author.toMap()[QStringLiteral("id")] == m_localUser->id()) {
                return true;
            }
        }
        return false;
    }

    return {};
}

int ReactionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_reactions.count();
}

void ReactionModel::setReactions(QList<Reaction> reactions)
{
    beginResetModel();
    m_reactions.clear();
    m_reactions = reactions;
    endResetModel();
}

QHash<int, QByteArray> ReactionModel::roleNames() const
{
    return {
        {TextRole, "text"},
        {ReactionRole, "reaction"},
        {ToolTipRole, "toolTip"},
        {AuthorsRole, "authors"},
        {HasLocalUser, "hasLocalUser"},
    };
}

#include "moc_reactionmodel.cpp"
