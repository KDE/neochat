// SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "sentryintegration.h"

#include <sentry.h>

using namespace Qt::Literals::StringLiterals;

Sentry &Sentry::instance()
{
    static Sentry _instance;
    return _instance;
}

Sentry::Sentry()
    : QObject()
{
    QString dsn = QStringLiteral("https://%1@crash-reports.kde.org/%2").arg("296a68fe1cf24ee79fafad735365d8d6"_L1, "18"_L1);
    auto options = sentry_options_new();
    sentry_options_set_dsn(options, dsn.toLatin1().data());
    sentry_options_set_release(options, "neochat@TESTING");
    sentry_options_set_debug(options, false);
    sentry_options_add_attachment(options, "/home/tobias/.local/share/KDE/neochat/neochat.log.0");
    sentry_init(options);
}

void Sentry::sendLogs()
{
    auto event = sentry_value_new_message_event(SENTRY_LEVEL_INFO, "custom", "It works!");
    auto uuid = sentry_capture_event(event);
    auto str = (char *)malloc(37);
    sentry_uuid_as_string(&uuid, str);
    qWarning() << "sent logs" << str;
}
