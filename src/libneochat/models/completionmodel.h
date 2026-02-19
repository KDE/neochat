// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QConcatenateTablesProxyModel>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSortFilterProxyModel>

#include "chatbarcache.h"
#include "chattextitemhelper.h"
#include "enums/chatbartype.h"
#include "neochatroom.h"

class CompletionProxyModel;
class UserListModel;
class RoomListModel;

/**
 * @class CompletionModel
 *
 * This class defines the model for suggesting completions in chat text.
 *
 * This model is able to select the appropriate completion type for the input text
 * and present a list of options that can be presented to the user.
 */
class CompletionModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QML text Item that completions are being provided for.
     */
    Q_PROPERTY(ChatTextItemHelper *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    /**
     * @brief The current type of completion being done on the entered text.
     *
     * @sa AutoCompletionType
     */
    Q_PROPERTY(AutoCompletionType autoCompletionType READ autoCompletionType NOTIFY autoCompletionTypeChanged)

    /**
     * @brief The RoomListModel to be used for room completions.
     */
    Q_PROPERTY(RoomListModel *roomListModel READ roomListModel WRITE setRoomListModel NOTIFY roomListModelChanged)

    /**
     * @brief The UserListModel to be used for room completions.
     */
    Q_PROPERTY(UserListModel *userListModel READ userListModel WRITE setUserListModel NOTIFY userListModelChanged)

    /**
     * @brief The UserListModel to be used for room completions.
     */
    Q_PROPERTY(bool isCompleting READ isCompleting NOTIFY isCompletingChanged)

public:
    /**
     * @brief Defines the different types of completion available.
     */
    enum AutoCompletionType {
        User, /**< A user in the current room. */
        Room, /**< A matrix room. */
        Emoji, /**< An emoji. */
        Command, /**< A / command. */
        None, /**< No available completion for the current text. */
    };
    Q_ENUM(AutoCompletionType)

    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        DisplayNameRole = Qt::DisplayRole, /**< The main text to show. */
        SubtitleRole = Qt::UserRole, /**< The subtitle text to show. */
        IconNameRole, /**< The icon to show. */
        ReplacedTextRole, /**< The text to replace the input text with for the completion. */
        HRefRole, /**< The hyperlink if applicable for the completion. */
    };
    Q_ENUM(Roles)

    explicit CompletionModel(QObject *parent = nullptr);

    ChatTextItemHelper *textItem() const;
    void setTextItem(ChatTextItemHelper *textItem);

    bool isCompleting() const;

    Q_INVOKABLE void ignoreCurrentCompletion();

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    RoomListModel *roomListModel() const;
    void setRoomListModel(RoomListModel *roomListModel);

    UserListModel *userListModel() const;
    void setUserListModel(UserListModel *userListModel);

    AutoCompletionType autoCompletionType() const;
    void setAutoCompletionType(AutoCompletionType autoCompletionType);

    Q_INVOKABLE void insertCompletion(const QString &text, const QUrl &link);

Q_SIGNALS:
    void textItemChanged();
    void autoCompletionTypeChanged();
    void roomListModelChanged();
    void userListModelChanged();
    void isCompletingChanged();

private:
    QPointer<ChatTextItemHelper> m_textItem;

    bool m_ignoreCurrentCompletion = false;
    int m_textStart = 0;
    void updateTextStart();

    CompletionProxyModel *m_filterModel;
    AutoCompletionType m_autoCompletionType = None;

    void updateCompletion();

    UserListModel *m_userListModel;
    RoomListModel *m_roomListModel;
    QConcatenateTablesProxyModel *m_emojiModel;
};
