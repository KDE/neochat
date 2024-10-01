// SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QJSValue>
#include <QObject>
#include <QQmlEngine>

/** Permission enum for use in QML. */
namespace Permission
{
Q_NAMESPACE
enum Permission {
    InvalidPermission, // captures QML errors resulting in "0" enum values
    ReadCalendar,
    WriteCalendar,
    PostNotification,
    Camera,
};
Q_ENUM_NS(Permission)
}

/** Check and request platform permissions for access to controlled resources (calendar, location, etc). */
class PermissionManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    static PermissionManager &instance();
    static PermissionManager *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    Q_INVOKABLE static bool checkPermission(Permission::Permission permission);
    Q_INVOKABLE static void requestPermission(Permission::Permission permission, QJSValue callback);

private:
    explicit PermissionManager(QObject *parent = nullptr);
};
