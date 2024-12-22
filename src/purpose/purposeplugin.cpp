// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <KIO/CommandLauncherJob>
#include <KPluginFactory>
#include <Purpose/PluginBase>

using namespace Qt::StringLiterals;

class NeoChatJob : public Purpose::Job
{
    Q_OBJECT
public:
    explicit NeoChatJob(QObject *parent)
        : Purpose::Job(parent)
    {
    }

    QStringList arrayToList(const QJsonArray &array)
    {
        QStringList ret;
        for (const auto &val : array) {
            ret += val.toString();
        }
        return ret;
    }

    void start() override
    {
        const QJsonArray urlsJson = data().value("urls"_L1).toArray();
        const QString title = data().value("title"_L1).toString();
        const QString message = u"%1 - %2"_s.arg(title, arrayToList(urlsJson).join(QLatin1Char(' ')));

        auto *job = new KIO::CommandLauncherJob(u"neochat"_s, {u"--share"_s, message});
        connect(job, &KJob::finished, this, &NeoChatJob::emitResult);
        job->start();
    }
};

class Q_DECL_EXPORT PurposePlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    PurposePlugin(QObject *p, const QVariantList &)
        : Purpose::PluginBase(p)
    {
    }

    Purpose::Job *createJob() const override
    {
        return new NeoChatJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(PurposePlugin, "purposeplugin.json")

#include "purposeplugin.moc"
