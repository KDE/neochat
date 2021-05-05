// SPDX-FileCopyrightText: 2021 Srevin Saju <srevinsaju@sugarlabs.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <KLocalizedString>

#include "actionshandler.h"
#include "commandmodel.h"


QVariantList CommandModel::filterModel(const QString &filter)
{
    QVariantList result;

    for (const QVariant &e : CommandModel::commands()) {
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


QVariantList CommandModel::commands()
{
    QVariantList commands;

    // Messages commands
    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/me "),
        QStringLiteral("<message>"),
        i18n("Displays action")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/shrug "),
        QStringLiteral("<message>"),
        i18n("Prepends ¯\\_(ツ)_/¯ to a plain-text message")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/lenny "),
        QStringLiteral("<message>"),
        i18n("Prepends ( ͡° ͜ʖ ͡°) to a plain-text message")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/plain "),
        QStringLiteral("<message>"),
        i18n("Sends a message as plain text, without interpreting it as markdown")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/html "),
        QStringLiteral("<message>"),
        i18n("Sends a message as html, without interpreting it as markdown")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/rainbow "),
        QStringLiteral("<message>"),
        i18n("Sends the given message coloured as a rainbow")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/rainbowme "),
        QStringLiteral("<message>"),
        i18n("Sends the given emote coloured as a rainbow")}));


    // Actions commands
    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/join "), QStringLiteral("<room-address>"),
        i18n("Joins room with given address")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/part "),
        QStringLiteral("[<room-address>]"),
        i18n("Leave room")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/invite "),
        QStringLiteral("<user-id>"),
        i18n("Invites user with given id to current room")}));

    commands.append(QVariant::fromValue(Command{
        QStringLiteral("/react "),
        QStringLiteral("<reaction text>"),
        i18n("React to this message with a text")}));

    // TODO more see elements /help action

    return commands;
}

