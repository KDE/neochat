// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "mediamanager.h"

#include <QDirIterator>
#include <QMimeDatabase>

#include <Quotient/qt_connection_util.h>

#include "events/callmemberevent.h"
#include "neochatroom.h"

using namespace Qt::Literals::StringLiterals;
using namespace Quotient;

void MediaManager::startPlayback()
{
    Q_EMIT playbackStarted();
}

void MediaManager::ring(const QJsonObject &json, NeoChatRoom *room)
{
    qWarning() << "start check ring";
    // todo: check sender != us
    if (json["content"_L1]["application"_L1].toString() != "m.call"_L1) {
        qWarning() << "not m.call";
        return;
    }
    qWarning() << json;
    if (!json["content"_L1]["m.mentions"_L1]["room"_L1].toBool() || json[u"sender"_s].toString() == room->connection()->userId()) {
        bool mentioned = false;
        for (const auto &user : json["content"_L1]["m.mentions"_L1]["user_ids"_L1].toArray()) {
            if (user.toString() == room->connection()->userId()) {
                mentioned = true;
                break;
            }
        }
        if (!mentioned) {
            qWarning() << "not mentioned";
            return;
        }
    }
    if (json["content"_L1]["notify_type"_L1].toString() != "ring"_L1) {
        qWarning() << "not ring";
        return;
    }
    if (room->pushNotificationState() == PushNotificationState::Mute) {
        qWarning() << "mute";
        return;
    }
    if (isRinging()) {
        qWarning() << "already ringing";
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
            if (event) {
                auto memberships = event->contentJson()["memberships"_L1].toArray();
                for (const auto &m : memberships) {
                    const auto &membership = m.toObject();
                    if (membership["application"_L1] == "m.call"_L1 && membership["call_id"_L1].toString().isEmpty()) {
                        qWarning() << "stopping";
                        stopRinging();
                        return true;
                    }
                }
            }
        }
        return false;
    });
    if (json["unsigned"_L1]["age"_L1].toInt() > 10000) {
        qWarning() << "too old";
        return;
    }
    ringUnchecked();
}

void MediaManager::ringUnchecked()
{
    qWarning() << "ring";
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
    Q_EMIT showIncomingCallDialog();
}

MediaManager::MediaManager(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer())
    , m_output(new QAudioOutput())
    , m_timer(new QTimer())
{
    m_player->setAudioOutput(m_output);
    m_timer->setInterval(1000);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_player->play();
    });
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, [this]() {
        if (m_player->playbackState() == QMediaPlayer::StoppedState) {
            m_timer->start();
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
    m_timer->stop();
    //Q_EMIT stopRinging();
}
