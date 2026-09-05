// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <qqmlintegration.h>

#include "blockcache.h"
#include "neochatroom.h"

class QMediaRecorder;

class PostMessageHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    Q_PROPERTY(Blocks::Cache *cache READ cache WRITE setCache NOTIFY cacheChanged)

    Q_PROPERTY(QString editId READ editId WRITE setEditId NOTIFY editIdChanged)

    Q_PROPERTY(QString threadRootId READ threadRootId WRITE setThreadRootId NOTIFY threadRootIdChanged)

public:
    explicit PostMessageHelper(QObject *parent = nullptr);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);
    Blocks::Cache *cache() const;
    void setCache(Blocks::Cache *cache);

    QString editId() const;
    void setEditId(const QString &editId);
    QString threadRootId() const;
    void setThreadRootId(const QString &threadRootId);

    Q_INVOKABLE void postMessage();

    Q_INVOKABLE void postPoll(PollKind::Kind kind, const QString &question, const QList<QString> &answers);

    Q_INVOKABLE void postVoiceMessage(QMediaRecorder *recorder);

Q_SIGNALS:
    void roomChanged();
    void cacheChanged();
    void editIdChanged();
    void threadRootIdChanged();

private:
    QPointer<NeoChatRoom> m_room;
    Blocks::Cache *m_cache = nullptr;
    QString m_editId = {};
    QString m_threadRootId = {};
};
