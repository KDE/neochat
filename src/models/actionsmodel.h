// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <KLazyLocalizedString>
#include <QAbstractListModel>
#include <events/roommessageevent.h>

class NeoChatRoom;

class ActionsModel : public QAbstractListModel
{
public:
    struct Action {
        // The prefix, without '/' and space after the word
        QString prefix;
        std::function<QString(const QString &, NeoChatRoom *)> handle;
        // If this is true, this action transforms a message to a different message and it will be sent.
        // If this is false, this message does some action on the client and should not be sent as a message.
        bool messageAction;
        // If this action changes the message type, this is the new message type. Otherwise it's nullopt
        std::optional<Quotient::RoomMessageEvent::MsgType> messageType = std::nullopt;
        KLazyLocalizedString parameters;
        KLazyLocalizedString description;
    };
    static ActionsModel &instance()
    {
        static ActionsModel _instance;
        return _instance;
    }

    enum Roles {
        Prefix = Qt::DisplayRole,
        Description,
        CompletionType,
        Parameters,
    };
    Q_ENUM(Roles);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QVector<Action> &allActions() const;

private:
    ActionsModel() = default;
};
