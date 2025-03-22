
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "roomsortparameter.h"

#include <algorithm>

// #include "neochatconfig.h"
#include <Integral/Utils>

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
    QList<RoomSortParameter::Parameter> configParamList = activitySortPriorities;
    // switch (static_cast<NeoChatConfig::EnumSortOrder::type>(NeoChatConfig::sortOrder())) {
    // case NeoChatConfig::EnumSortOrder::Activity:
    //     configParamList = activitySortPriorities;
    //     break;
    // case NeoChatConfig::EnumSortOrder::Alphabetical:
    //     configParamList = alphabeticalSortPriorities;
    //     break;
    // case NeoChatConfig::EnumSortOrder::LastMessage:
    //     configParamList = lastMessageSortPriorities;
    //     break;
    // case NeoChatConfig::EnumSortOrder::Custom: {
    //     const auto intList = NeoChatConfig::customSortOrder();
    //     std::transform(intList.constBegin(), intList.constEnd(), std::back_inserter(configParamList), [](int param) {
    //         return static_cast<Parameter>(param);
    //     });
    //     break;
    // }
    // default:
    //     break;
    // }

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
    // NeoChatConfig::setCustomSortOrder(intList);
    // NeoChatConfig::setSortOrder(NeoChatConfig::EnumSortOrder::Custom);
    // NeoChatConfig::self()->save();
}

int RoomSortParameter::compareParameter(Parameter parameter, rust::Box<sdk::RoomListRoom> leftRoom, rust::Box<sdk::RoomListRoom> rightRoom)
{
    switch (parameter) {
    case AlphabeticalAscending:
        return compareParameter<AlphabeticalAscending>(leftRoom->box_me(), rightRoom->box_me());
    case AlphabeticalDescending:
        return compareParameter<AlphabeticalDescending>(leftRoom->box_me(), rightRoom->box_me());
    case HasUnread:
        return compareParameter<HasUnread>(leftRoom->box_me(), rightRoom->box_me());
    case MostUnread:
        return compareParameter<MostUnread>(leftRoom->box_me(), rightRoom->box_me());
    case HasHighlight:
        return compareParameter<HasHighlight>(leftRoom->box_me(), rightRoom->box_me());
    case MostHighlights:
        return compareParameter<MostHighlights>(leftRoom->box_me(), rightRoom->box_me());
    case LastActive:
        return compareParameter<LastActive>(leftRoom->box_me(), rightRoom->box_me());
    default:
        return 0;
    }
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::AlphabeticalAscending>(rust::Box<sdk::RoomListRoom> leftRoom, rust::Box<sdk::RoomListRoom> rightRoom)
{
    return -typeCompare(stringFromRust(leftRoom->display_name()), stringFromRust(rightRoom->display_name()));
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::AlphabeticalDescending>(rust::Box<sdk::RoomListRoom> leftRoom,
                                                                                   rust::Box<sdk::RoomListRoom> rightRoom)
{
    return typeCompare(stringFromRust(leftRoom->display_name()), stringFromRust(rightRoom->display_name()));
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::HasUnread>(rust::Box<sdk::RoomListRoom> leftRoom, rust::Box<sdk::RoomListRoom> rightRoom)
{
    return typeCompare(leftRoom->num_unread_messages() > 0, rightRoom->num_unread_messages() > 0);
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::MostUnread>(rust::Box<sdk::RoomListRoom> leftRoom, rust::Box<sdk::RoomListRoom> rightRoom)
{
    return typeCompare(leftRoom->num_unread_messages(), rightRoom->num_unread_messages());
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::HasHighlight>(rust::Box<sdk::RoomListRoom> leftRoom, rust::Box<sdk::RoomListRoom> rightRoom)
{
    const auto leftHighlight = leftRoom->num_unread_mentions() > 0 && leftRoom->num_unread_messages() > 0;
    const auto rightHighlight = rightRoom->num_unread_mentions() > 0 && rightRoom->num_unread_messages() > 0;
    return typeCompare(leftHighlight, rightHighlight);
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::MostHighlights>(rust::Box<sdk::RoomListRoom> leftRoom, rust::Box<sdk::RoomListRoom> rightRoom)
{
    return typeCompare(int(leftRoom->num_unread_mentions()), int(rightRoom->num_unread_mentions()));
}

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::LastActive>(rust::Box<sdk::RoomListRoom> leftRoom, rust::Box<sdk::RoomListRoom> rightRoom)
{
    return 1;
}
