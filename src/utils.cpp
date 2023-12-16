// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "utils.h"

using namespace Quotient;

static const QVariantMap emptyUser = {
    {"isLocalUser"_ls, false},
    {"id"_ls, QString()},
    {"displayName"_ls, QString()},
    {"avatarSource"_ls, QUrl()},
    {"avatarMediaId"_ls, QString()},
    {"color"_ls, QColor()},
    {"object"_ls, QVariant()},
};

QVariantMap QmlUtils::getUser(User *user) const
{
    if (user == nullptr) {
        return emptyUser;
    }

    const auto &url = user->avatarUrl();
    if (url.isEmpty() || url.scheme() != "mxc"_ls) {
        return {};
    }
    auto avatarSource = user->connection()->makeMediaUrl(url);
    if (!avatarSource.isValid() || avatarSource.scheme() != QStringLiteral("mxc")) {
        avatarSource = {};
    }

    return QVariantMap{
        {QStringLiteral("isLocalUser"), user->id() == user->connection()->user()->id()},
        {QStringLiteral("id"), user->id()},
        {QStringLiteral("displayName"), user->displayname()},
        {QStringLiteral("escapedDisplayName"), user->displayname().toHtmlEscaped()},
        {QStringLiteral("avatarSource"), avatarSource},
        {QStringLiteral("avatarMediaId"), user->avatarMediaId()},
        {QStringLiteral("color"), Utils::getUserColor(user->hueF())},
        {QStringLiteral("object"), QVariant::fromValue(user)},
    };
}
