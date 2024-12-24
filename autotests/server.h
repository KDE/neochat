// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QHttpServer>
#include <QSslServer>

class Server
{
public:
    Server();

    void start();
private:
    QHttpServer m_server;
    QSslServer m_sslServer;
};
