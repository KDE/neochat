// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QConcatenateTablesProxyModel>
#include <QSortFilterProxyModel>

#include "roomlistmodel.h"

class CompletionProxyModel;
class UserListModel;
class NeoChatRoom;
class RoomListModel;

class CompletionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)
    Q_PROPERTY(AutoCompletionType autoCompletionType READ autoCompletionType NOTIFY autoCompletionTypeChanged);
    Q_PROPERTY(RoomListModel *roomListModel READ roomListModel WRITE setRoomListModel NOTIFY roomListModelChanged);

public:
    enum AutoCompletionType {
        User,
        Room,
        Emoji,
        Command,
        None,
    };
    Q_ENUM(AutoCompletionType)

    enum Roles {
        Text = Qt::DisplayRole,
        Subtitle,
        Icon,
        ReplacedText,
    };
    Q_ENUM(Roles);

    CompletionModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString text() const;
    void setText(const QString &text, const QString &fullText);
    void updateCompletion();

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    RoomListModel *roomListModel() const;
    void setRoomListModel(RoomListModel *roomListModel);

    AutoCompletionType autoCompletionType() const;

Q_SIGNALS:
    void textChanged();
    void roomChanged();
    void autoCompletionTypeChanged();
    void roomListModelChanged();

private:
    QString m_text;
    QString m_fullText;
    CompletionProxyModel *m_filterModel;
    NeoChatRoom *m_room = nullptr;
    AutoCompletionType m_autoCompletionType = None;

    void setAutoCompletionType(AutoCompletionType autoCompletionType);

    UserListModel *m_userListModel;
    RoomListModel *m_roomListModel;
    QConcatenateTablesProxyModel *m_emojiModel;
};
Q_DECLARE_METATYPE(CompletionModel::AutoCompletionType);
