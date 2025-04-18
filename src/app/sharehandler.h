// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

class ShareHandler : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString room READ room WRITE setRoom NOTIFY roomChanged)

public:
    static ShareHandler &instance()
    {
        static ShareHandler _instance;
        return _instance;
    }

    static ShareHandler *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    QString text() const;
    void setText(const QString &url);

    QString room() const;
    void setRoom(const QString &roomId);

Q_SIGNALS:
    void textChanged();
    void roomChanged();

private:
    explicit ShareHandler(QObject *parent = nullptr);

    QString m_text;
    QString m_room;
};
