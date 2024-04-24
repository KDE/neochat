// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "proxycontroller.h"

#include <QNetworkProxy>
#include <QNetworkProxyFactory>

#include "neochatconfig.h"

void ProxyController::setApplicationProxy()
{
    auto cfg = NeoChatConfig::self();
    QNetworkProxy proxy;

    switch (cfg->proxyType()) {
    case 1:
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(cfg->proxyHost());
        proxy.setPort(cfg->proxyPort());
        proxy.setUser(cfg->proxyUser());
        proxy.setPassword(cfg->proxyPassword());
        QNetworkProxy::setApplicationProxy(proxy);
        break;
    case 2:
        proxy.setType(QNetworkProxy::Socks5Proxy);
        proxy.setHostName(cfg->proxyHost());
        proxy.setPort(cfg->proxyPort());
        proxy.setUser(cfg->proxyUser());
        proxy.setPassword(cfg->proxyPassword());
        QNetworkProxy::setApplicationProxy(proxy);
        break;
    default:
        QNetworkProxyFactory::setUseSystemConfiguration(true);
        break;
    }
}

ProxyController::ProxyController(QObject *parent)
    : QObject(parent)
{
}

#include "moc_proxycontroller.cpp"
