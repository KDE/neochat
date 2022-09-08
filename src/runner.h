// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDBusContext>
#include <QObject>

#include <QDBusArgument>
#include <QList>
#include <QString>
#include <QVariantMap>

#include "roomlistmodel.h"
#include "sortfilterroomlistmodel.h"

// Copied from KRunner/QueryMatch
enum MatchType {
    NoMatch = 0, /**< Null match */
    CompletionMatch = 10, /**< Possible completion for the data of the query */
    PossibleMatch = 30, /**< Something that may match the query */
    InformationalMatch = 50, /**< A purely informational, non-runnable match,
                                such as the answer to a question or calculation.
                                The data of the match will be converted to a string
                                and set in the search field */
    HelperMatch = 70, /**< A match that represents an action not directly related
                            to activating the given search term, such as a search
                            in an external tool or a command learning trigger. Helper
                            matches tend to be generic to the query and should not
                            be autoactivated just because the user hits "Enter"
                            while typing. They must be explicitly selected to
                            be activated, but unlike InformationalMatch cause
                            an action to be triggered. */
    ExactMatch = 100, /**< An exact match to the query */
};

struct RemoteMatch {
    // sssuda{sv}
    QString id;
    QString text;
    QString iconName;
    MatchType type = MatchType::NoMatch;
    qreal relevance = 0;
    QVariantMap properties;
};

typedef QList<RemoteMatch> RemoteMatches;

struct RemoteAction {
    QString id;
    QString text;
    QString iconName;
};

typedef QList<RemoteAction> RemoteActions;

struct RemoteImage {
    // iiibiiay (matching notification spec image-data attribute)
    int width;
    int height;
    int rowStride;
    bool hasAlpha;
    int bitsPerSample;
    int channels;
    QByteArray data;
};

inline QDBusArgument &operator<<(QDBusArgument &argument, const RemoteMatch &match)
{
    argument.beginStructure();
    argument << match.id;
    argument << match.text;
    argument << match.iconName;
    argument << match.type;
    argument << match.relevance;
    argument << match.properties;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteMatch &match)
{
    argument.beginStructure();
    argument >> match.id;
    argument >> match.text;
    argument >> match.iconName;
    uint type;
    argument >> type;
    match.type = static_cast<MatchType>(type);
    argument >> match.relevance;
    argument >> match.properties;
    argument.endStructure();

    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const RemoteAction &action)
{
    argument.beginStructure();
    argument << action.id;
    argument << action.text;
    argument << action.iconName;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteAction &action)
{
    argument.beginStructure();
    argument >> action.id;
    argument >> action.text;
    argument >> action.iconName;
    argument.endStructure();
    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const RemoteImage &image)
{
    argument.beginStructure();
    argument << image.width;
    argument << image.height;
    argument << image.rowStride;
    argument << image.hasAlpha;
    argument << image.bitsPerSample;
    argument << image.channels;
    argument << image.data;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, RemoteImage &image)
{
    argument.beginStructure();
    argument >> image.width;
    argument >> image.height;
    argument >> image.rowStride;
    argument >> image.hasAlpha;
    argument >> image.bitsPerSample;
    argument >> image.channels;
    argument >> image.data;
    argument.endStructure();
    return argument;
}

Q_DECLARE_METATYPE(RemoteMatch)
Q_DECLARE_METATYPE(RemoteMatches)
Q_DECLARE_METATYPE(RemoteAction)
Q_DECLARE_METATYPE(RemoteActions)
Q_DECLARE_METATYPE(RemoteImage)

class Runner : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.krunner1")
public:
    Runner();

    Q_SCRIPTABLE RemoteActions Actions();
    Q_SCRIPTABLE RemoteMatches Match(const QString &searchTerm);
    Q_SCRIPTABLE void Run(const QString &id, const QString &actionId);

private:
    RemoteImage serializeImage(const QImage &image);
    void activeConnectionChanged();

    SortFilterRoomListModel m_model;
    RoomListModel m_sourceModel;
};
