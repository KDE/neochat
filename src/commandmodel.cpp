// SPDX-FileCopyrightText: 2021 Srevin Saju <srevinsaju@sugarlabs.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>

#include "commandmodel.h"
#include "actionshandler.h"


QVariantList CommandModel::filterModel(const QString &filter)
{
    QVariantList result;

    for (const QVariant &e : ActionsHandler::commands()) {
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
