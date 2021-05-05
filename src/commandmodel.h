// SPDX-FileCopyrightText: 2021 Srevin Saju <srevinsaju@sugarlabs.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QVector>
#include <utility>

struct Command {
    Command(const QString &p, const QString &a, const QString &h)
        : command(p)
        , parameter(a)
        , help(h)
    {
    }
    Command() = default;

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

    Q_INVOKABLE QVariantList filterModel(const QString &filter);
    static QVariantList commands();
};
