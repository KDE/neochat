// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "neochatconnection.h"

class SupportContact
{
    Q_GADGET

    Q_PROPERTY(QString role MEMBER role)
    Q_PROPERTY(QString matrixId MEMBER matrixId)
    Q_PROPERTY(QString emailAddress MEMBER emailAddress)

public:
    QString role;
    QString matrixId;
    QString emailAddress;
};

class SupportController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(NeoChatConnection *connection READ connection WRITE setConnection NOTIFY connectionChanged REQUIRED)
    Q_PROPERTY(QString supportPage READ supportPage NOTIFY loaded)
    Q_PROPERTY(QList<SupportContact> contacts READ contacts NOTIFY loaded)

public:
    void setConnection(NeoChatConnection *connection);
    NeoChatConnection *connection() const;

    QString supportPage() const;
    QList<SupportContact> contacts() const;

Q_SIGNALS:
    void connectionChanged();
    void loaded();

private:
    void load();

    QPointer<NeoChatConnection> m_connection = nullptr;
    QList<SupportContact> m_contacts;
    QString m_supportPage;
};
