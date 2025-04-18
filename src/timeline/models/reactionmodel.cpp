// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "reactionmodel.h"
#include "utils.h"

#include <QDebug>
#include <QFont>

#include <KLocalizedString>

#include "models/messagecontentmodel.h"
#include "neochatroom.h"

using namespace Qt::StringLiterals;

ReactionModel::ReactionModel(MessageContentModel *parent, const QString &eventId, NeoChatRoom *room)
    : QAbstractListModel(parent)
    , m_room(room)
    , m_eventId(eventId)
{
    Q_ASSERT(parent);
    Q_ASSERT(parent != nullptr);
    Q_ASSERT(!eventId.isEmpty());
    Q_ASSERT(room != nullptr);

    connect(m_room, &NeoChatRoom::updatedEvent, this, [this](const QString &eventId) {
        if (m_eventId == eventId) {
            updateReactions();
        }
    });

    updateReactions();
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

    if (role == TextContentRole) {
        if (reaction.authors.count() > 1) {
            return u"%1  %2"_s.arg(reactionText(reaction.reaction), QString::number(reaction.authors.count()));
        } else {
            return reactionText(reaction.reaction);
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
                    text += u", "_s;
                } else {
                    text += i18nc("Separate the usernames of users", " and ");
                }
            }
            text += m_room->member(reaction.authors.at(i)).displayName();
        }

        if (reaction.authors.count() > 3) {
            text += i18ncp("%1 is the number of other users", " and %1 other", " and %1 others", reaction.authors.count() - 3);
        }

        text = i18ncp("%2 is the users who reacted and %3 the emoji that was given",
                      "%2 reacted with %3",
                      "%2 reacted with %3",
                      reaction.authors.count(),
                      text,
                      m_shortcodes.contains(reaction.reaction) ? m_shortcodes[reaction.reaction] : reactionText(reaction.reaction));
        return text;
    }

    if (role == HasLocalMember) {
        for (auto author : reaction.authors) {
            if (author == m_room->localMember().id()) {
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

void ReactionModel::updateReactions()
{
    if (m_room == nullptr) {
        return;
    }

    beginResetModel();

    m_reactions.clear();
    m_shortcodes.clear();

    const auto &annotations = m_room->relatedEvents(m_eventId, Quotient::EventRelation::AnnotationType);
    if (annotations.isEmpty()) {
        endResetModel();
        return;
    };

    QMap<QString, QStringList> reactions = {};
    for (const auto &a : annotations) {
        if (a->isRedacted()) { // Just in case?
            continue;
        }
        if (const auto &e = eventCast<const Quotient::ReactionEvent>(a)) {
            reactions[e->key()].append(e->senderId());
            if (e->contentJson()["shortcode"_L1].toString().length()) {
                m_shortcodes[e->key()] = e->contentJson()["shortcode"_L1].toString().toHtmlEscaped();
            }
        }
    }

    if (reactions.isEmpty()) {
        endResetModel();
        return;
    }
    auto i = reactions.constBegin();
    while (i != reactions.constEnd()) {
        QStringList members;
        for (const auto &member : i.value()) {
            members.append(member);
        }

        m_reactions.append(ReactionModel::Reaction{i.key(), members});
        ++i;
    }

    endResetModel();
}

QHash<int, QByteArray> ReactionModel::roleNames() const
{
    return {
        {TextContentRole, "textContent"},
        {ReactionRole, "reaction"},
        {ToolTipRole, "toolTip"},
        {HasLocalMember, "hasLocalMember"},
    };
}

QString ReactionModel::reactionText(QString text) const
{
    text = text.toHtmlEscaped();
    if (text.startsWith(u"mxc://"_s)) {
        static QFont font;
        static int size = font.pixelSize();
        if (size == -1) {
            size = font.pointSizeF() * 1.333;
        }
        return u"<img src=\"%1\" width=\"%2\" height=\"%2\">"_s.arg(m_room->connection()->makeMediaUrl(QUrl(text)).toString(), QString::number(size));
    }

    return Utils::isEmoji(text) ? u"<span style=\"font-family: 'emoji';\">"_s + text + u"</span>"_s : text;
}

#include "moc_reactionmodel.cpp"
