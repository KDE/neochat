// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messageattached.h"

MessageAttached::MessageAttached(QObject *parent)
    : QQuickAttachedPropertyPropagator(parent)
{
    if (parent == nullptr) {
        qWarning() << "Message must be attached to an Item" << parent;
        return;
    }
    initialize();
}

MessageAttached *MessageAttached::qmlAttachedProperties(QObject *object)
{
    return new MessageAttached(object);
}

NeoChatRoom *MessageAttached::room() const
{
    return m_room;
}

void MessageAttached::setRoom(NeoChatRoom *room)
{
    m_explicitRoom = true;
    if (m_room == room) {
        return;
    }
    m_room = room;
    propagateMessage(this);
    Q_EMIT roomChanged();
}

QQuickItem *MessageAttached::timeline() const
{
    return m_timeline;
}

void MessageAttached::setTimeline(QQuickItem *timeline)
{
    m_explicitTimeline = true;
    if (m_timeline == timeline) {
        return;
    }
    m_timeline = timeline;
    propagateMessage(this);
    Q_EMIT timelineChanged();
}

int MessageAttached::index() const
{
    return m_index;
}

void MessageAttached::setIndex(int index)
{
    m_explicitIndex = true;
    if (m_index == index) {
        return;
    }
    m_index = index;
    propagateMessage(this);
    Q_EMIT indexChanged();
}

qreal MessageAttached::maxContentWidth() const
{
    return m_maxContentWidth;
}

void MessageAttached::setMaxContentWidth(qreal maxContentWidth)
{
    m_explicitMaxContentWidth = true;
    if (m_maxContentWidth == maxContentWidth) {
        return;
    }
    m_maxContentWidth = maxContentWidth;
    propagateMessage(this);
    Q_EMIT maxContentWidthChanged();
}

void MessageAttached::propagateMessage(MessageAttached *message)
{
    if (m_explicitRoom || m_room != message->room()) {
        m_room = message->room();
        Q_EMIT roomChanged();
    }

    if (m_explicitTimeline || m_timeline != message->timeline()) {
        m_timeline = message->timeline();
        Q_EMIT timelineChanged();
    }

    if (m_explicitIndex || m_index != message->index()) {
        m_index = message->index();
        Q_EMIT indexChanged();
    }

    if (m_explicitMaxContentWidth || m_maxContentWidth != message->maxContentWidth()) {
        m_maxContentWidth = message->maxContentWidth();
        Q_EMIT maxContentWidthChanged();
    }

    const auto styles = attachedChildren();
    for (auto *child : attachedChildren()) {
        MessageAttached *message = qobject_cast<MessageAttached *>(child);
        if (message != nullptr) {
            message->propagateMessage(this);
        }
    }
}

void MessageAttached::attachedParentChange(QQuickAttachedPropertyPropagator *newParent, QQuickAttachedPropertyPropagator *oldParent)
{
    Q_UNUSED(oldParent);

    MessageAttached *attachedParent = qobject_cast<MessageAttached *>(newParent);
    if (attachedParent) {
        propagateMessage(attachedParent);
    }
}

#include "moc_messageattached.cpp"
