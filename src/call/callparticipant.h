// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>
#include <QTimer>

#include "neochatuser.h"

class QQuickItem;

class CallParticipant : public QObject
{
    Q_OBJECT
    Q_PROPERTY(NeoChatUser *user READ user CONSTANT)
    Q_PROPERTY(bool hasCamera READ hasCamera NOTIFY hasCameraChanged)

public:
    NeoChatUser *m_user = nullptr;
    bool m_hasCamera = false;

    Q_INVOKABLE void initCamera(QQuickItem *item);

    [[nodiscard]] NeoChatUser *user() const;

    [[nodiscard]] bool hasCamera() const;

    explicit CallParticipant(QObject *parent = nullptr);

Q_SIGNALS:
    void initialized(QQuickItem *item);
    void heightChanged();
    void widthChanged();
    void hasCameraChanged();
};