// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "proxycontroller.h"

#include <QNetworkProxy>

#include "neochatconfig.h"

void ProxyController::setApplicationProxy()
{
    auto cfg = NeoChatConfig::self();
    QNetworkProxy proxy;

    switch (cfg->proxyType()) {
    case 1:
        proxy.setType(QNetworkProxy::HttpProxy);
        break;
    case 2:
        proxy.setType(QNetworkProxy::Socks5Proxy);
        break;
    default:
        break;
    }
    proxy.setHostName(cfg->proxyHost());
    proxy.setPort(cfg->proxyPort());
    proxy.setUser(cfg->proxyUser());
    proxy.setPassword(cfg->proxyPassword());
    QNetworkProxy::setApplicationProxy(proxy);
}

ProxyController::ProxyController(QObject *parent)
    : QObject(parent)
{
}
