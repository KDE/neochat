// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomsortparameter.h"

#include <algorithm>

#include "neochatconfig.h"
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

QList<RoomSortParameter::Parameter> RoomSortParameter::allParameterList()
{
    return allSortPriorities;
}

QList<RoomSortParameter::Parameter> RoomSortParameter::currentParameterList()
{
    QList<RoomSortParameter::Parameter> configParamList;
    switch (static_cast<NeoChatConfig::EnumSortOrder::type>(NeoChatConfig::sortOrder())) {
    case NeoChatConfig::EnumSortOrder::Activity:
        configParamList = activitySortPriorities;
        break;
    case NeoChatConfig::EnumSortOrder::Alphabetical:
        configParamList = alphabeticalSortPriorities;
        break;
    case NeoChatConfig::EnumSortOrder::LastMessage:
        configParamList = lastMessageSortPriorities;
        break;
    case NeoChatConfig::EnumSortOrder::Custom: {
        const auto intList = NeoChatConfig::customSortOrder();
        std::transform(intList.constBegin(), intList.constEnd(), std::back_inserter(configParamList), [](int param) {
            return static_cast<Parameter>(param);
        });
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

void RoomSortParameter::saveNewParameterList(const QList<Parameter> &newList)
{
    QList<int> intList;
    std::transform(newList.constBegin(), newList.constEnd(), std::back_inserter(intList), [](Parameter param) {
        return static_cast<int>(param);
    });
    NeoChatConfig::setCustomSortOrder(intList);
    NeoChatConfig::setSortOrder(NeoChatConfig::EnumSortOrder::Custom);
    NeoChatConfig::self()->save();
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
