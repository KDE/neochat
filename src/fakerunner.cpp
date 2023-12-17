// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "fakerunner.h"

#include <QCoreApplication>
#include <QDBusMetaType>

Q_SCRIPTABLE RemoteActions FakeRunner::Actions()
{
    QCoreApplication::quit();
    return {};
}

Q_SCRIPTABLE RemoteMatches FakeRunner::Match(const QString &searchTerm)
{
    QCoreApplication::quit();
    return {};
}

Q_SCRIPTABLE void FakeRunner::Run(const QString &id, const QString &actionId)
{
    QCoreApplication::quit();
}

FakeRunner::FakeRunner()
    : QObject()
{
    qDBusRegisterMetaType<RemoteMatch>();
    qDBusRegisterMetaType<RemoteMatches>();
    qDBusRegisterMetaType<RemoteAction>();
    qDBusRegisterMetaType<RemoteActions>();
    qDBusRegisterMetaType<RemoteImage>();
}

#include "moc_fakerunner.cpp"