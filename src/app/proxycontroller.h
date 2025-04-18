// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QObject>
#include <QQmlEngine>

class ProxyController : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static ProxyController &instance()
    {
        static ProxyController _instance;
        return _instance;
    }
    static ProxyController *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    /**
     * @brief Sets the QNetworkProxy for the application.
     *
     * @sa QNetworkProxy::setApplicationProxy
     */
    Q_INVOKABLE void setApplicationProxy();

private:
    explicit ProxyController(QObject *parent = nullptr);
};
