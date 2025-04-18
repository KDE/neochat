// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QDBusContext>

#include "runner.h"

/**
 * This is a close-to-identical copy of the regular Runner interface,
 * only used when activated for push notifications. This stubs it out so
 * Plasma Search and Kickoff doesn't accidentally activate the push notification
 * service.
 *
 * @sa Runner
 */
class FakeRunner : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.krunner1")

public:
    Q_SCRIPTABLE RemoteActions Actions();

    Q_SCRIPTABLE RemoteMatches Match(const QString &searchTerm);

    Q_SCRIPTABLE void Run(const QString &id, const QString &actionId);

    FakeRunner();
};