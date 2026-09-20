// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include "completionlist.h"
#include "models/roomlistmodel.h"
#include "models/userlistmodel.h"

class UserCompletionList : public CompletionList
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The UserListModel to adapt for completions.
     */
    Q_PROPERTY(UserListModel *userListModel READ userListModel WRITE setUserListModel NOTIFY userListModelChanged)

public:
    explicit UserCompletionList(QObject *parent = nullptr);

    UserListModel *userListModel() const;
    void setUserListModel(UserListModel *userListModel);

    qsizetype size() const override;

    QVariant data(qsizetype row, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void userListModelChanged();

private:
    QPointer<UserListModel> m_userListModel;
};

class RoomCompletionList : public CompletionList
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The RoomListModel to adapt for completions.
     */
    Q_PROPERTY(RoomListModel *roomListModel READ roomListModel WRITE setRoomListModel NOTIFY roomListModelChanged)

public:
    explicit RoomCompletionList(QObject *parent = nullptr);

    RoomListModel *roomListModel() const;
    void setRoomListModel(RoomListModel *roomListModel);

    qsizetype size() const override;

    QVariant data(qsizetype row, int role = Qt::DisplayRole) const override;

Q_SIGNALS:
    void roomListModelChanged();

private:
    QPointer<RoomListModel> m_roomListModel;
};

class ActionsCompletionList : public CompletionList
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ActionsCompletionList(QObject *parent = nullptr);

    qsizetype size() const override;

    QVariant data(qsizetype row, int role = Qt::DisplayRole) const override;
};

class EmojiCompletionList : public CompletionList
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit EmojiCompletionList(QObject *parent = nullptr);

    qsizetype size() const override;

    QVariant data(qsizetype row, int role = Qt::DisplayRole) const override;
};

class CustomEmojiCompletionList : public CompletionList
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CustomEmojiCompletionList(QObject *parent = nullptr);

    qsizetype size() const override;

    QVariant data(qsizetype row, int role = Qt::DisplayRole) const override;
};
