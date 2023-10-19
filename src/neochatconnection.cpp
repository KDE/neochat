// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "neochatconnection.h"

#include <QImageReader>

#include "controller.h"
#include "jobs/neochatchangepasswordjob.h"
#include "jobs/neochatdeactivateaccountjob.h"
#include "roommanager.h"

#include <qt6keychain/keychain.h>

#include <KLocalizedString>

#include <Quotient/csapi/content-repo.h>
#include <Quotient/csapi/profile.h>
#include <Quotient/qt_connection_util.h>
#include <Quotient/settings.h>
#include <Quotient/user.h>

using namespace Quotient;

NeoChatConnection::NeoChatConnection(QObject *parent)
    : Connection(parent)
{
    connect(this, &NeoChatConnection::accountDataChanged, this, [this](const QString &type) {
        if (type == QLatin1String("org.kde.neochat.account_label")) {
            Q_EMIT labelChanged();
        }
    });
}

NeoChatConnection::NeoChatConnection(const QUrl &server, QObject *parent)
    : Connection(server, parent)
{
    connect(this, &NeoChatConnection::accountDataChanged, this, [this](const QString &type) {
        if (type == QLatin1String("org.kde.neochat.account_label")) {
            Q_EMIT labelChanged();
        }
    });
}

void NeoChatConnection::logout(bool serverSideLogout)
{
    SettingsGroup(QStringLiteral("Accounts")).remove(userId());

    QKeychain::DeletePasswordJob job(qAppName());
    job.setAutoDelete(true);
    job.setKey(userId());
    QEventLoop loop;
    QKeychain::DeletePasswordJob::connect(&job, &QKeychain::Job::finished, &loop, &QEventLoop::quit);
    job.start();
    loop.exec();

    if (Controller::instance().accounts().count() > 1) {
        // Only set the connection if the the account being logged out is currently active
        if (this == Controller::instance().activeConnection()) {
            Controller::instance().setActiveConnection(dynamic_cast<NeoChatConnection *>(Controller::instance().accounts().accounts()[0]));
        }
    } else {
        Controller::instance().setActiveConnection(nullptr);
    }
    if (!serverSideLogout) {
        return;
    }
    Connection::logout();
}

bool NeoChatConnection::setAvatar(const QUrl &avatarSource)
{
    QString decoded = avatarSource.path();
    if (decoded.isEmpty()) {
        callApi<SetAvatarUrlJob>(user()->id(), avatarSource);
        return true;
    }
    if (QImageReader(decoded).read().isNull()) {
        return false;
    } else {
        return user()->setAvatar(decoded);
    }
}

QVariantList NeoChatConnection::getSupportedRoomVersions() const
{
    const auto &roomVersions = availableRoomVersions();
    QVariantList supportedRoomVersions;
    for (const auto &v : roomVersions) {
        QVariantMap roomVersionMap;
        roomVersionMap.insert("id"_ls, v.id);
        roomVersionMap.insert("status"_ls, v.status);
        roomVersionMap.insert("isStable"_ls, v.isStable());
        supportedRoomVersions.append(roomVersionMap);
    }
    return supportedRoomVersions;
}

void NeoChatConnection::changePassword(const QString &currentPassword, const QString &newPassword)
{
    auto job = callApi<NeochatChangePasswordJob>(newPassword, false);
    connect(job, &BaseJob::result, this, [this, job, currentPassword, newPassword] {
        if (job->error() == 103) {
            QJsonObject replyData = job->jsonData();
            QJsonObject authData;
            authData["session"_ls] = replyData["session"_ls];
            authData["password"_ls] = currentPassword;
            authData["type"_ls] = "m.login.password"_ls;
            authData["user"_ls] = user()->id();
            QJsonObject identifier = {{"type"_ls, "m.id.user"_ls}, {"user"_ls, user()->id()}};
            authData["identifier"_ls] = identifier;
            NeochatChangePasswordJob *innerJob = callApi<NeochatChangePasswordJob>(newPassword, false, authData);
            connect(innerJob, &BaseJob::success, this, []() {
                Q_EMIT Controller::instance().passwordStatus(Controller::PasswordStatus::Success);
            });
            connect(innerJob, &BaseJob::failure, this, [innerJob]() {
                Q_EMIT Controller::instance().passwordStatus(innerJob->jsonData()["errcode"_ls] == "M_FORBIDDEN"_ls ? Controller::PasswordStatus::Wrong
                                                                                                                    : Controller::PasswordStatus::Other);
            });
        }
    });
}

void NeoChatConnection::setLabel(const QString &label)
{
    QJsonObject json{
        {"account_label"_ls, label},
    };
    setAccountData("org.kde.neochat.account_label"_ls, json);
    Q_EMIT labelChanged();
}

QString NeoChatConnection::label() const
{
    return accountDataJson("org.kde.neochat.account_label"_ls)["account_label"_ls].toString();
}

void NeoChatConnection::deactivateAccount(const QString &password)
{
    auto job = callApi<NeoChatDeactivateAccountJob>();
    connect(job, &BaseJob::result, this, [this, job, password] {
        if (job->error() == 103) {
            QJsonObject replyData = job->jsonData();
            QJsonObject authData;
            authData["session"_ls] = replyData["session"_ls];
            authData["password"_ls] = password;
            authData["type"_ls] = "m.login.password"_ls;
            authData["user"_ls] = user()->id();
            QJsonObject identifier = {{"type"_ls, "m.id.user"_ls}, {"user"_ls, user()->id()}};
            authData["identifier"_ls] = identifier;
            auto innerJob = callApi<NeoChatDeactivateAccountJob>(authData);
            connect(innerJob, &BaseJob::success, this, [this]() {
                logout(false);
            });
        }
    });
}

void NeoChatConnection::createRoom(const QString &name, const QString &topic, const QString &parent, bool setChildParent)
{
    QList<CreateRoomJob::StateEvent> initialStateEvents;
    if (!parent.isEmpty()) {
        initialStateEvents.append(CreateRoomJob::StateEvent{
            "m.space.parent"_ls,
            QJsonObject{
                {"canonical"_ls, true},
                {"via"_ls, QJsonArray{domain()}},
            },
            parent,
        });
    }

    const auto job = Connection::createRoom(Connection::PublishRoom, QString(), name, topic, QStringList(), {}, {}, {}, initialStateEvents);
    if (!parent.isEmpty()) {
        connect(job, &Quotient::CreateRoomJob::success, this, [this, parent, setChildParent, job]() {
            if (setChildParent) {
                if (auto parentRoom = room(parent)) {
                    parentRoom->setState(QLatin1String("m.space.child"), job->roomId(), QJsonObject{{QLatin1String("via"), QJsonArray{domain()}}});
                }
            }
        });
    }
    connect(job, &CreateRoomJob::failure, this, [this, job] {
        Q_EMIT Controller::instance().errorOccured(i18n("Room creation failed: %1", job->errorString()));
    });
    connectSingleShot(this, &Connection::newRoom, this, [](Room *room) {
        RoomManager::instance().enterRoom(dynamic_cast<NeoChatRoom *>(room));
    });
}

void NeoChatConnection::createSpace(const QString &name, const QString &topic, const QString &parent, bool setChildParent)
{
    QList<CreateRoomJob::StateEvent> initialStateEvents;
    if (!parent.isEmpty()) {
        initialStateEvents.append(CreateRoomJob::StateEvent{
            "m.space.parent"_ls,
            QJsonObject{
                {"canonical"_ls, true},
                {"via"_ls, QJsonArray{domain()}},
            },
            parent,
        });
    }

    const auto job = Connection::createRoom(Connection::UnpublishRoom, {}, name, topic, {}, {}, {}, false, initialStateEvents, {}, QJsonObject{{"type"_ls, "m.space"_ls}});
    if (!parent.isEmpty()) {
        connect(job, &Quotient::CreateRoomJob::success, this, [this, parent, setChildParent, job]() {
            if (setChildParent) {
                if (auto parentRoom = room(parent)) {
                    parentRoom->setState(QLatin1String("m.space.child"), job->roomId(), QJsonObject{{QLatin1String("via"), QJsonArray{domain()}}});
                }
            }
        });
    }
    connect(job, &CreateRoomJob::failure, this, [this, job] {
        Q_EMIT Controller::instance().errorOccured(i18n("Space creation failed: %1", job->errorString()));
    });
    connectSingleShot(this, &Connection::newRoom, this, [](Room *room) {
        RoomManager::instance().enterRoom(dynamic_cast<NeoChatRoom *>(room));
    });
}

#include "moc_neochatconnection.cpp"
