// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "reactionmodel.h"
#include "neochatroom.h"

#include <QDebug>
#ifdef HAVE_ICU
#include <QTextBoundaryFinder>
#include <QTextCharFormat>
#include <unicode/uchar.h>
#include <unicode/urename.h>
#endif

#include <KLocalizedString>

#include <Quotient/user.h>

ReactionModel::ReactionModel(const Quotient::RoomMessageEvent *event, NeoChatRoom *room)
    : QAbstractListModel(nullptr)
    , m_room(room)
    , m_event(event)
{
    if (m_event != nullptr && m_room != nullptr) {
        connect(m_room, &NeoChatRoom::updatedEvent, this, [this](const QString &eventId) {
            if (m_event && m_event->id() == eventId) {
                updateReactions();
            }
        });

        updateReactions();
    }
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
            return QStringLiteral("%1  %2").arg(reactionText(reaction.reaction), QString::number(reaction.authors.count()));
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
                      m_shortcodes.contains(reaction.reaction) ? m_shortcodes[reaction.reaction] : reactionText(reaction.reaction));
        return text;
    }

    if (role == AuthorsRole) {
        return reaction.authors;
    }

    if (role == HasLocalUser) {
        for (auto author : reaction.authors) {
            if (author.toMap()[QStringLiteral("id")] == m_room->localUser()->id()) {
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
    beginResetModel();

    m_reactions.clear();
    m_shortcodes.clear();

    const auto &annotations = m_room->relatedEvents(*m_event, Quotient::EventRelation::AnnotationType);
    if (annotations.isEmpty()) {
        endResetModel();
        return;
    };

    QMap<QString, QList<Quotient::User *>> reactions = {};
    for (const auto &a : annotations) {
        if (a->isRedacted()) { // Just in case?
            continue;
        }
        if (const auto &e = eventCast<const Quotient::ReactionEvent>(a)) {
            reactions[e->key()].append(m_room->user(e->senderId()));
            if (e->contentJson()[QStringLiteral("shortcode")].toString().length()) {
                m_shortcodes[e->key()] = e->contentJson()[QStringLiteral("shortcode")].toString().toHtmlEscaped();
            }
        }
    }

    if (reactions.isEmpty()) {
        endResetModel();
        return;
    }

    auto i = reactions.constBegin();
    while (i != reactions.constEnd()) {
        QVariantList authors;
        for (const auto &author : i.value()) {
            authors.append(m_room->getUser(author));
        }

        m_reactions.append(ReactionModel::Reaction{i.key(), authors});
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
        {AuthorsRole, "authors"},
        {HasLocalUser, "hasLocalUser"},
    };
}

bool isEmoji(const QString &text)
{
#ifdef HAVE_ICU
    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
    int from = 0;
    while (finder.toNextBoundary() != -1) {
        auto to = finder.position();
        if (text[from].isSpace()) {
            from = to;
            continue;
        }

        auto first = text.mid(from, to - from).toUcs4()[0];
        if (!u_hasBinaryProperty(first, UCHAR_EMOJI)) {
            return false;
        }
        from = to;
    }
    return true;
#else
    return false;
#endif
}

QString ReactionModel::reactionText(QString text) const
{
    text = text.toHtmlEscaped();
    if (text.startsWith(QStringLiteral("mxc://"))) {
        static QFont font;
        static int size = font.pixelSize();
        if (size == -1) {
            size = font.pointSizeF() * 1.333;
        }
        return QStringLiteral("<img src=\"%1\" width=\"%2\" height=\"%2\">")
            .arg(m_room->connection()->makeMediaUrl(QUrl(text)).toString(), QString::number(size));
    }

    return isEmoji(text) ? QStringLiteral("<span style=\"font-family: 'emoji';\">") + text + QStringLiteral("</span>") : text;
}

#include "moc_reactionmodel.cpp"
