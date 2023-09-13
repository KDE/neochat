// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "runner.h"

#include <QDBusMetaType>

#include "controller.h"
#include "neochatroom.h"
#include "roommanager.h"
#include "windowcontroller.h"

RemoteImage Runner::serializeImage(const QImage &image)
{
    QImage convertedImage = image.convertToFormat(QImage::Format_RGBA8888);
    RemoteImage remoteImage{
        convertedImage.width(),
        convertedImage.height(),
        static_cast<int>(convertedImage.bytesPerLine()),
        true, // hasAlpha
        8, // bitsPerSample
        4, // channels
        QByteArray(reinterpret_cast<const char *>(convertedImage.constBits()), convertedImage.sizeInBytes()),
    };
    return remoteImage;
}

Runner::Runner()
    : QObject()
{
    qDBusRegisterMetaType<RemoteMatch>();
    qDBusRegisterMetaType<RemoteMatches>();
    qDBusRegisterMetaType<RemoteAction>();
    qDBusRegisterMetaType<RemoteActions>();
    qDBusRegisterMetaType<RemoteImage>();

    m_model.setSourceModel(&m_sourceModel);

    connect(&Controller::instance(), &Controller::activeConnectionChanged, this, &Runner::activeConnectionChanged);
}

void Runner::activeConnectionChanged()
{
    m_sourceModel.setConnection(Controller::instance().activeConnection());
}

RemoteActions Runner::Actions()
{
    return {};
}

RemoteMatches Runner::Match(const QString &searchTerm)
{
    m_model.setFilterText(searchTerm);

    RemoteMatches matches;

    for (int i = 0; i < m_model.rowCount(); ++i) {
        RemoteMatch match;

        const QString name = m_model.data(m_model.index(i, 0), RoomListModel::DisplayNameRole).toString();

        match.iconName = QStringLiteral("org.kde.neochat");
        match.id = m_model.data(m_model.index(i, 0), RoomListModel::RoomIdRole).toString();
        match.text = name;
        match.relevance = 1;
        const RemoteImage remoteImage = serializeImage(m_model.data(m_model.index(i, 0), RoomListModel::AvatarImageRole).value<QImage>());
        match.properties.insert(QStringLiteral("icon-data"), QVariant::fromValue(remoteImage));
        match.properties.insert(QStringLiteral("subtext"), m_model.data(m_model.index(i, 0), RoomListModel::TopicRole).toString());

        if (name.compare(searchTerm, Qt::CaseInsensitive) == 0) {
            match.type = ExactMatch;
        } else {
            match.type = CompletionMatch;
        }

        matches << match;
    }

    return matches;
}

void Runner::Run(const QString &id, const QString &actionId)
{
    Q_UNUSED(actionId);

    NeoChatRoom *room = qobject_cast<NeoChatRoom *>(Controller::instance().activeConnection()->room(id));

    if (!room) {
        return;
    }

    RoomManager::instance().enterRoom(room);

    WindowController::instance().showAndRaiseWindow(QString());
}

#include "moc_runner.cpp"
