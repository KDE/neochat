// SPDX-FileCopyrightText: 2021 Srevin Saju <srevinsaju@sugarlabs.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QVector>
#include <utility>

struct Command {
    Command(QString p, QString a, QString h)
        : command(std::move(std::move(p)))
        , parameter(std::move(std::move(a)))
        , help(std::move(std::move(h)))
    {
    }
    Command() = default;

    friend QDataStream &operator<<(QDataStream &arch, const Command &object)
    {
        arch << object.command;
        arch << object.parameter;
        arch << object.help;
        return arch;
    }

    friend QDataStream &operator>>(QDataStream &arch, Command &object)
    {
        arch >> object.command;
        arch >> object.parameter;
        arch >> object.help;
        return arch;
    }

    QString command;
    QString parameter;
    QString help;

Q_GADGET
    Q_PROPERTY(QString command MEMBER command)
    Q_PROPERTY(QString parameter MEMBER parameter)
    Q_PROPERTY(QString help MEMBER help)
};

Q_DECLARE_METATYPE(Command)

class CommandModel : public QObject
{
Q_OBJECT

public:
    explicit CommandModel(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    Q_INVOKABLE static QVariantList filterModel(const QString &filter);

};
