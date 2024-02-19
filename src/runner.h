// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QDBusContext>
#include <QObject>

#include <QDBusArgument>
#include <QList>
#include <QString>
#include <QVariantMap>

#include "models/roomlistmodel.h"
#include "models/sortfilterroomlistmodel.h"

/**
 * The type of match. Value is important here as it is used for sorting
 *
 * Copied from KRunner/QueryMatch
 *
 * @sa KRunner/QueryMatch
 */
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
    qsizetype rowStride;
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

/**
 * @class Runner
 *
 * A class to define the NeoChat KRunner plugin.
 *
 * @sa KRunner
 */
class Runner : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.krunner1")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(RoomListModel *roomListModel READ roomListModel WRITE setRoomListModel NOTIFY roomListModelChanged)
public:
    static Runner *create(QQmlEngine *engine, QJSEngine *)
    {
        static Runner instance;
        engine->setObjectOwnership(&instance, QQmlEngine::CppOwnership);
        return &instance;
    }

    /**
     * @brief Return a list of KRunner actions.
     *
     * @note It's always empty; nothing is broken.
     */
    Q_SCRIPTABLE RemoteActions Actions();

    /**
     * @brief Return a list of room matches for a search.
     */
    Q_SCRIPTABLE RemoteMatches Match(const QString &searchTerm);

    /**
     * @brief Handle action calls.
     */
    Q_SCRIPTABLE void Run(const QString &id, const QString &actionId);

    void setRoomListModel(RoomListModel *roomListModel);
    RoomListModel *roomListModel() const;

Q_SIGNALS:
    void roomListModelChanged();

private:
    RemoteImage serializeImage(const QImage &image);

    SortFilterRoomListModel m_model;
    RoomListModel *m_sourceModel;
    Runner();
};
