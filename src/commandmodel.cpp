// SPDX-FileCopyrightText: 2021 Srevin Saju <srevinsaju@sugarlabs.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>

#include "commandmodel.h"


QVariantList CommandModel::filterModel(const QString &filter)
{
    QVariantList result;

    for (const QVariant &e : matrix) {
        auto command = qvariant_cast<Command>(e);
        if (command.command.startsWith(filter)) {
            result.append(e);
            if (result.length() > 10) {
                return result;
            }
        }
    }

    return result;
}


// the help messages are taken from Element (web matrix client, app.element.io)
const QVariantList CommandModel::matrix = {
    QVariant::fromValue(Command{"/join", "Join a given room with address"}),
    QVariant::fromValue(Command{"/me", "Displays action"}),

};
