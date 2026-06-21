// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentmodel.h"

#include <QStyleHints>

#include <KLocalizedString>

#include "block.h"
#include "chatbarcache.h"
#include "messagecontentlogging.h"
#include "neochatdatetime.h"
#include "texthandler.h"

using namespace Quotient;

std::function<void(const QString &, bool)> MessageContentModel::m_setMediaHidden = [](const QString &, bool) {

};

std::function<bool(const QString &)> MessageContentModel::m_mediaShouldBeHidden = [](const QString &) -> bool {
    return false;
};

MessageContentModel::MessageContentModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

MessageContentModel::MessageContentModel(NeoChatRoom *room, const QString &eventId, MessageContentModel *parent)
    : QAbstractListModel(parent)
    , m_eventId(eventId)
{
    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &MessageContentModel::updateSpoilers);

    m_mediaHidden = m_mediaShouldBeHidden(m_eventId);

    setRoom(room);
}

NeoChatRoom *MessageContentModel::room() const
{
    return m_room;
}

void MessageContentModel::setRoom(NeoChatRoom *room)
{
    if (room == m_room) {
        return;
    }

    if (m_room) {
        m_room->disconnect(this);
    }

    const auto oldRoom = std::exchange(m_room, room);

    if (m_room) {
        connect(m_room, &NeoChatRoom::urlPreviewEnabledChanged, this, &MessageContentModel::componentsUpdated);
    }

    Q_EMIT roomChanged(oldRoom, m_room);
}

QString MessageContentModel::eventId() const
{
    return m_eventId;
}

NeoChatDateTime MessageContentModel::dateTime() const
{
    return QDateTime::currentDateTime();
}

QString MessageContentModel::authorId() const
{
    return m_room->localMember().id();
}

NeochatRoomMember *MessageContentModel::author() const
{
    return m_room->qmlSafeMember(authorId());
}

QString MessageContentModel::threadRootId() const
{
    return {};
}

QVariant MessageContentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        qCWarning(MessageContent) << __FUNCTION__ << "called with invalid index" << index << rowCount();
        return {};
    }

    if (!m_room) {
        qCWarning(MessageContent) << __FUNCTION__ << "called without room";
        return {};
    }

    const auto &component = m_components[index.row()];
    if (!component) {
        return {};
    }

    if (role == ComponentTypeRole) {
        return component->type();
    }
    if (role == BlockRole) {
        return component->toVariant();
    }
    if (role == EventIdRole) {
        return eventId();
    }
    if (role == DateTimeRole) {
        return QVariant::fromValue(dateTime());
    }
    if (role == AuthorRole) {
        return QVariant::fromValue<NeochatRoomMember *>(author());
    }
    if (role == ReplyContentModelRole) {
        return QVariant::fromValue<MessageContentModel *>(m_replyModel);
    }
    if (role == ThreadRootRole) {
        return threadRootId();
    }
    if (role == EditableRole) {
        return m_editableActive;
    }
    if (role == CurrentFocusRole) {
        return index.row() == m_currentFocusComponent.row();
    }
    if (role == MediaHiddenRole) {
        return m_mediaHidden;
    }

    return {};
}

int MessageContentModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_components.size();
}

QHash<int, QByteArray> MessageContentModel::roleNames() const
{
    return roleNamesStatic();
}

QHash<int, QByteArray> MessageContentModel::roleNamesStatic()
{
    QHash<int, QByteArray> roles;
    roles[MessageContentModel::ComponentTypeRole] = "componentType";
    roles[MessageContentModel::BlockRole] = "block";
    roles[MessageContentModel::EventIdRole] = "eventId";
    roles[MessageContentModel::DateTimeRole] = "dateTime";
    roles[MessageContentModel::AuthorRole] = "author";
    roles[MessageContentModel::ReplyContentModelRole] = "replyContentModel";
    roles[MessageContentModel::ThreadRootRole] = "threadRoot";
    roles[MessageContentModel::EditableRole] = "editable";
    roles[MessageContentModel::CurrentFocusRole] = "currentFocus";
    roles[MessageContentModel::MediaHiddenRole] = "mediaHidden";
    return roles;
}

bool MessageContentModel::hasComponentType(Blocks::Type type) const
{
    return std::find_if(m_components.cbegin(),
                        m_components.cend(),
                        [type](Blocks::Block *component) {
                            return component->type() == type;
                        })
        != m_components.cend();
}

bool MessageContentModel::hasComponentType(const QList<Blocks::Type> &types) const
{
    return std::ranges::any_of(types, [this](const Blocks::Type &type) {
        return hasComponentType(type);
    });
}

void MessageContentModel::forEachComponentOfType(Blocks::Type type, std::function<Blocks::BlockPtrsIt(Blocks::BlockPtrsIt)> function)
{
    auto it = m_components.begin();
    while ((it = std::find_if(it,
                              m_components.end(),
                              [type](const Blocks::Block *component) {
                                  return component->type() == type;
                              }))
           != m_components.end()) {
        it = function(it);
    }
}

void MessageContentModel::forEachComponentOfType(QList<Blocks::Type> types, std::function<Blocks::BlockPtrsIt(Blocks::BlockPtrsIt)> function)
{
    for (const auto &type : types) {
        forEachComponentOfType(type, function);
    }
}

void MessageContentModel::updateSpoilers()
{
    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        updateSpoiler(index(it - m_components.begin()));
    }
}

void MessageContentModel::updateSpoiler(const QModelIndex &index)
{
    const auto row = index.row();
    if (row < 0 || row >= rowCount()) {
        qCWarning(MessageContent) << __FUNCTION__ << "called with invalid index" << index << rowCount();
        return;
    }

    const auto textBlock = dynamic_cast<Blocks::TextBlock *>(m_components[row]);
    if (!textBlock) {
        return;
    }
    const auto item = textBlock->item();
    if (!item) {
        return;
    }
    const auto doc = item->document();
    if (!doc) {
        return;
    }

    const auto newText = TextHandler::updateSpoilerText(this, doc->toHtml(), textBlock->spoilerRevealed());
    doc->clear();
    doc->setHtml(newText);
    Q_EMIT dataChanged(index, index, {BlockRole});
}

void MessageContentModel::toggleSpoiler(QModelIndex index)
{
    const auto row = index.row();
    if (row < 0 || row >= rowCount()) {
        qCWarning(MessageContent) << __FUNCTION__ << "called with invalid row" << row << m_components.size();
        return;
    }
    const auto textBlock = dynamic_cast<Blocks::TextBlock *>(m_components[row]);
    if (!textBlock) {
        return;
    }

    textBlock->setSpoilerRevealed(!textBlock->spoilerRevealed());
    Q_EMIT dataChanged(index, index, {BlockRole});
    updateSpoiler(index);
}

void MessageContentModel::hideMedia()
{
    m_mediaHidden = true;
    Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {MediaHiddenRole});

    m_setMediaHidden(m_eventId, true);
}

void MessageContentModel::showMedia()
{
    m_mediaHidden = false;
    Q_EMIT dataChanged(index(0, 0), index(rowCount() - 1, 0), {MediaHiddenRole});

    m_setMediaHidden(m_eventId, false);
}

bool MessageContentModel::isMediaHidden()
{
    return m_mediaHidden;
}

void MessageContentModel::setSetMediaHidden(std::function<void(const QString &, bool)> func)
{
    m_setMediaHidden = func;
}

void MessageContentModel::setMediaShouldBeHidden(std::function<bool(const QString &)> func)
{
    m_mediaShouldBeHidden = func;
}

#include "moc_messagecontentmodel.cpp"
