// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "roomsortorder.h"
#include <QObject>
#include <QQmlEngine>

#include <KLocalizedString>

class NeoChatRoom;

/**
 * @class RoomSortParameter
 *
 * A class with the Parameter enum for room sort parameters.
 */
class RoomSortParameter : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief Defines the available sort parameters.
     *
     * @note All values are specifically numbered as they should never change even
     *       if new options are later added. This is because they are stored in
     *       the config as ints and changing would break someones config on upgrade.
     */
    enum Parameter {
        AlphabeticalAscending = 0,
        AlphabeticalDescending = 1,
        HasUnread = 2,
        MostUnread = 3,
        HasHighlight = 4,
        MostHighlights = 5,
        LastActive = 6,
    };
    Q_ENUM(Parameter)

    /**
     * @brief Translate the Parameter enum value to a human readable name string.
     *
     * @sa Parameter
     */
    Q_INVOKABLE static QString parameterName(RoomSortParameter::Parameter parameter)
    {
        switch (parameter) {
        case Parameter::AlphabeticalAscending:
            return i18nc("As in sorting alphabetically with A first and Z last", "Alphabetical Ascending");
        case Parameter::AlphabeticalDescending:
            return i18nc("As in sorting alphabetically with Z first and A last", "Alphabetical Descending");
        case Parameter::HasUnread:
            return i18nc("As in sorting rooms with unread message above those without", "Has Unread Messages");
        case Parameter::MostUnread:
            return i18nc("As in sorting rooms with the most unread messages higher", "Most Unread Messages");
        case Parameter::HasHighlight:
            return i18nc("As in sorting rooms with highlighted message above those without", "Has Highlighted Messages");
        case Parameter::MostHighlights:
            return i18nc("As in sorting rooms with the most highlighted messages higher", "Most Highlighted Messages");
        case Parameter::LastActive:
            return i18nc("As in sorting the chat room with the newest meassage first", "Last Active");
        default:
            return {};
        }
    };

    /**
     * @brief Translate the Parameter enum value to a human readable description string.
     *
     * @sa Parameter
     */
    Q_INVOKABLE static QString parameterDescription(RoomSortParameter::Parameter parameter)
    {
        switch (parameter) {
        case Parameter::AlphabeticalAscending:
            return i18nc("@info", "Room names closer to A alphabetically are higher");
        case Parameter::AlphabeticalDescending:
            return i18nc("@info", "Room names closer to Z alphabetically are higher");
        case Parameter::HasUnread:
            return i18nc("@info", "Rooms with unread messages are higher");
        case Parameter::MostUnread:
            return i18nc("@info", "Rooms with the most unread message are higher");
        case Parameter::HasHighlight:
            return i18nc("@info", "Rooms with highlighted messages are higher");
        case Parameter::MostHighlights:
            return i18nc("@info", "Rooms with the most highlighted messages are higher");
        case Parameter::LastActive:
            return i18nc("@info", "Rooms with newer events are higher");
        default:
            return {};
        }
    };

    /**
     * @brief List of all available Parameter sort orders.
     */
    static QList<Parameter> allParameterList();

    /**
     * @brief The current Parameter sort order list.
     */
    static QList<Parameter> currentParameterList();

    /**
     * @brief Compare the given parameter of the two given rooms.
     *
     * @return 0 if they are equal, 1 if the left is greater and -1 if the right is greater.
     *
     * @sa Parameter
     */
    static int compareParameter(Parameter parameter, NeoChatRoom *leftRoom, NeoChatRoom *rightRoom);

    static void setSortOrder(RoomSortOrder::Order order);
    static void setCustomSortOrder(QList<Parameter> order);

private:
    static RoomSortOrder::Order m_sortOrder;
    static QList<Parameter> m_customSortOrder;

    template<Parameter parameter>
    static int compareParameter(NeoChatRoom *, NeoChatRoom *)
    {
        return false;
    }
};

template<>
int RoomSortParameter::compareParameter<RoomSortParameter::AlphabeticalAscending>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom);
template<>
int RoomSortParameter::compareParameter<RoomSortParameter::AlphabeticalDescending>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom);
template<>
int RoomSortParameter::compareParameter<RoomSortParameter::HasUnread>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom);
template<>
int RoomSortParameter::compareParameter<RoomSortParameter::MostUnread>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom);
template<>
int RoomSortParameter::compareParameter<RoomSortParameter::HasHighlight>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom);
template<>
int RoomSortParameter::compareParameter<RoomSortParameter::MostHighlights>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom);
template<>
int RoomSortParameter::compareParameter<RoomSortParameter::LastActive>(NeoChatRoom *leftRoom, NeoChatRoom *rightRoom);
