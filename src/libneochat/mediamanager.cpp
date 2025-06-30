// SPDX-FileCopyrightText: 2023-2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "mediamanager.h"

#include <QAudioOutput>
#include <QDBusConnection>
#include <QDirIterator>
#include <QMediaPlayer>
#include <QMimeDatabase>

#include <Quotient/qt_connection_util.h>

#include "events/callmemberevent.h"
#include "mediaplayer2player.h"
#include "neochatroom.h"

using namespace Qt::Literals::StringLiterals;
using namespace Quotient;

void MediaManager::startPlayback()
{
    Q_EMIT playbackStarted();
}

void MediaManager::ring(const QJsonObject &json, NeoChatRoom *room)
{
    // todo: check sender != us
    if (json["content"_L1]["application"_L1].toString() != "m.call"_L1) {
        return;
    }
    if (!json["content"_L1]["m.mentions"_L1]["room"_L1].toBool() || json["sender"_L1].toString() == room->connection()->userId()) {
        if (std::ranges::none_of(json["content"_L1]["m.mentions"_L1]["user_ids"_L1].toArray(), [room](const auto &user) {
                return user.toString() == room->connection()->userId();
            })) {
            return;
        }
    }
    if (json["content"_L1]["notify_type"_L1].toString() != "ring"_L1) {
        return;
    }
    if (room->pushNotificationState() == PushNotificationState::Mute) {
        return;
    }
    if (isRinging()) {
        return;
    }
    if (const auto &event = room->currentState().get<CallMemberEvent>(room->connection()->userId())) {
        if (event) {
            auto memberships = event->contentJson()["memberships"_L1].toArray();
            for (const auto &m : memberships) {
                const auto &membership = m.toObject();
                if (membership["application"_L1] == "m.call"_L1 && membership["call_id"_L1].toString().isEmpty()) {
                    qWarning() << "already in a call";
                    return;
                }
            }
        }
    }
    connectUntil(room, &NeoChatRoom::changed, this, [this, room]() {
        if (const auto &event = room->currentState().get<CallMemberEvent>(room->connection()->userId())) {
            auto memberships = event->contentJson()["memberships"_L1].toArray();
            for (const auto &m : memberships) {
                const auto &membership = m.toObject();
                if (membership["application"_L1] == "m.call"_L1 && membership["call_id"_L1].toString().isEmpty()) {
                    stopRinging();
                    return true;
                }
            }
        }
        return false;
    });
    if (json["unsigned"_L1]["age"_L1].toInt() > 10000) {
        return;
    }

    QTimer::singleShot(60000, this, &MediaManager::stopRinging);
    ringUnchecked();
}

void MediaManager::ringUnchecked()
{
    // Pause all media players registered with the system
    for (const auto &iface : QDBusConnection::sessionBus().interface()->registeredServiceNames().value()) {
        if (iface.startsWith("org.mpris.MediaPlayer2"_L1)) {
            OrgMprisMediaPlayer2PlayerInterface mprisInterface(iface, "/org/mpris/MediaPlayer2"_L1, QDBusConnection::sessionBus());
            QString status = mprisInterface.playbackStatus();
            if (status == "Playing"_L1) {
                if (mprisInterface.canPause()) {
                    mprisInterface.Pause();
                } else {
                    mprisInterface.Stop();
                }
            }
        }
    }

    static QString path;
    if (path.isEmpty()) {
        for (const auto &dir : QString::fromUtf8(qgetenv("XDG_DATA_DIRS")).split(u':')) {
            if (QFileInfo(dir + QStringLiteral("/sounds/freedesktop/stereo/phone-incoming-call.oga")).exists()) {
                path = dir + QStringLiteral("/sounds/freedesktop/stereo/phone-incoming-call.oga");
                break;
            }
        }
    }
    if (path.isEmpty()) {
        return;
    }

    m_player->setSource(QUrl::fromLocalFile(path));
    m_player->play();
}

MediaManager::MediaManager()
    : QObject(nullptr)
    , m_player(new QMediaPlayer())
    , m_output(new QAudioOutput())
{
    m_player->setAudioOutput(m_output);
    m_timer.setInterval(1000);
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        m_player->play();
    });
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this]() {
        if (m_player->playbackState() == QMediaPlayer::StoppedState) {
            m_timer.start();
        }
    });
}

bool MediaManager::isRinging() const
{
    return m_ringing;
}

void MediaManager::stopRinging()
{
    m_ringing = false;
    m_player->pause();
    m_timer.stop();
    Q_EMIT isRingingChanged();
}

#include "moc_mediamanager.cpp"
