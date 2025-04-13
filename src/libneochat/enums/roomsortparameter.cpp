// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomsortparameter.h"

#include <algorithm>

#include "enums/roomsortorder.h"
#include "neochatroom.h"

namespace
{
template<typename T>
int typeCompare(T left, T right)
{
    return left == right ? 0 : left > right ? 1 : -1;
}

template<>
int typeCompare<QString>(QString left, QString right)
{
    return left.localeAwareCompare(right);
}

static const QList<RoomSortParameter::Parameter> allSortPriorities = {
    RoomSortParameter::AlphabeticalAscending,
    RoomSortParameter::AlphabeticalDescending,
    RoomSortParameter::HasUnread,
    RoomSortParameter::MostUnread,
    RoomSortParameter::HasHighlight,
    RoomSortParameter::MostHighlights,
    RoomSortParameter::LastActive,
};

static const QList<RoomSortParameter::Parameter> alphabeticalSortPriorities = {
    RoomSortParameter::AlphabeticalAscending,
};

static const QList<RoomSortParameter::Parameter> activitySortPriorities = {
    RoomSortParameter::HasHighlight,
    RoomSortParameter::MostHighlights,
    RoomSortParameter::HasUnread,
    RoomSortParameter::MostUnread,
    RoomSortParameter::LastActive,
};

static const QList<RoomSortParameter::Parameter> lastMessageSortPriorities = {
    RoomSortParameter::LastActive,
};
}

RoomSortOrder::Order RoomSortParameter::m_sortOrder = RoomSortOrder::Activity;
QList<RoomSortParameter::Parameter> RoomSortParameter::m_customSortOrder = activitySortPriorities;

QList<RoomSortParameter::Parameter> RoomSortParameter::allParameterList()
{
    return allSortPriorities;
}

QList<RoomSortParameter::Parameter> RoomSortParameter::currentParameterList()
{
    QList<RoomSortParameter::Parameter> configParamList;
    switch (m_sortOrder) {
    case RoomSortOrder::Activity:
        configParamList = activitySortPriorities;
        break;
    case RoomSortOrder::Alphabetical:
        configParamList = alphabeticalSortPriorities;
        break;
    case RoomSortOrder::LastMessage:
        configParamList = lastMessageSortPriorities;
        break;
    case RoomSortOrder::Custom: {
        configParamList = m_customSortOrder;
        break;
    }
    default:
        break;
    }

    if (configParamList.isEmpty()) {
        return activitySortPriorities;
    }
    return configParamList;
}

int RoomSortParameter::compareParameter(Parameter parameter, NeoChatRoom *leftRoom, NeoChatRoom *rightRoom)
{
    switch (parameter) {
    case AlphabeticalAscending:
        return compareParameter<AlphabeticalAscending>(leftRoom, rightRoom);
    case AlphabeticalDescending:
        return compareParameter<AlphabeticalDescending>(leftRoom, rightRoom);
    case HasUnread:
        return compareParameter<HasUnread>(leftRoom, rightRoom);
    case MostUnread:
        return compareParameter<MostUnread>(leftRoom, rightRoom);
    case HasHighlight:
        return compareParameter<HasHighlight>(leftRoom, rightRoom);
    case MostHighlights:
        return compareParameter<MostHighlights>(leftRoom, rightRoom);
    case LastActive:
        return compareParameter<LastActive>(leftRoom, rightRoom);
    default:
        return 0;
    }
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::AlphabeticalAscending>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom)
{
    return -typeCompare(leftRoom->displayName(), rightRoom->displayName());
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::AlphabeticalDescending>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom)
{
    return typeCompare(leftRoom->displayName(), rightRoom->displayName());
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::HasUnread>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom)
{
    return typeCompare(leftRoom->contextAwareNotificationCount() > 0, rightRoom->contextAwareNotificationCount() > 0);
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::MostUnread>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom)
{
    return typeCompare(leftRoom->contextAwareNotificationCount(), rightRoom->contextAwareNotificationCount());
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::HasHighlight>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom)
{
    const auto leftHighlight = leftRoom->highlightCount() > 0 && leftRoom->contextAwareNotificationCount() > 0;
    const auto rightHighlight = rightRoom->highlightCount() > 0 && rightRoom->contextAwareNotificationCount() > 0;
    return typeCompare(leftHighlight, rightHighlight);
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::MostHighlights>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom)
{
    return typeCompare(int(leftRoom->highlightCount()), int(rightRoom->highlightCount()));
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::LastActive>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom)
{
    return typeCompare(leftRoom->lastActiveTime(), rightRoom->lastActiveTime());
}

void RoomSortParameter::setSortOrder(RoomSortOrder::Order order)
{
    RoomSortParameter::m_sortOrder = order;
}

void RoomSortParameter::setCustomSortOrder(QList<Parameter> order)
{
    RoomSortParameter::m_customSortOrder = order;
}
