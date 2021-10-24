// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <purpose/pluginbase.h>

#include <KApplicationTrader>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QDesktopServices>
#include <QJsonArray>
#include <QProcess>
#include <QStandardPaths>
#include <QUrl>
#include <QUrlQuery>

EXPORT_SHARE_VERSION

namespace
{
class NeoChatShareJob : public Purpose::Job
{
    Q_OBJECT
public:
    explicit NeoChatShareJob(QObject *parent = nullptr)
        : Purpose::Job(parent)
    {
    }

    void start() override
    {
    }
};
}

class Q_DECL_EXPORT NeoChatPurposePlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    NeoChatPurposePlugin(QObject *parent, const QVariantList &)
        : Purpose::PluginBase(parent)
    {
    }

    Purpose::Job *createJob() const override
    {
        return new NeoChatShareJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(NeoChatPurposePlugin, "neochatpurposeplugin.json")

#include "neochatpurposeplugin.moc"

