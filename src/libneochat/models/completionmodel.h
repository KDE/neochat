// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QConcatenateTablesProxyModel>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSortFilterProxyModel>

#include "chattextitemhelper.h"

class CompletionProxyModel;
class UserListModel;

class CompletionProvider : public QObject
{
public:
    using QObject::QObject;
    virtual bool matchesPrefix(QStringView text) const = 0;
    virtual QAbstractItemModel *model() const = 0;
    virtual QString textWithoutPrefix(const QString &text) const = 0;
};

class CompletionModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The UserFilterModel to be used for room completions.
     */
    Q_PROPERTY(UserListModel *userListModel READ userListModel WRITE setUserListModel NOTIFY userListModelChanged)
    /**
     * @brief The QML text Item that completions are being provided for.
     */
    Q_PROPERTY(ChatTextItemHelper *textItem READ textItem WRITE setTextItem NOTIFY textItemChanged)

    Q_PROPERTY(bool isCompleting READ isCompleting NOTIFY isCompletingChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum CompletionRoles {
        DisplayNameRole = Qt::UserRole + 100, /**< The main text to show. */
        SubtitleRole, /**< The subtitle text to show. */
        IconNameRole, /**< The icon to show. */
        ReplacedTextRole, /**< The text to replace the input text with for the completion. */
        HRefRole, /**< The hyperlink for the completion (if applicable). */
    };
    Q_ENUM(CompletionRoles);

    explicit CompletionModel(QObject *parent = nullptr);

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa EventRoles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    std::optional<CompletionProvider *> providerForText(QStringView text) const;

    UserListModel *userListModel() const;
    void setUserListModel(UserListModel *userListModel);

    ChatTextItemHelper *textItem() const;
    void setTextItem(ChatTextItemHelper *textItem);

    bool isCompleting() const;

    Q_INVOKABLE void ignoreCurrentCompletion();
    Q_INVOKABLE void insertCompletion(const QString &text, const QUrl &link);

Q_SIGNALS:
    void textItemChanged();
    void isCompletingChanged();
    void userListModelChanged();

private:
    QList<CompletionProvider *> m_providers;
    QPointer<ChatTextItemHelper> m_textItem;
    void updateCompletion();
    bool m_ignoreCurrentCompletion = false;
    int m_textStart = 0;
    void updateTextStart();
    CompletionProvider *m_userCompletionProvider = nullptr;
};

// User, /**< A user in the current room. */
// Room, /**< A matrix room. */
// Emoji, /**< An emoji. */
// Command, /**< A / command. */
// None, /**< No available completion for the current text. */

// class CompletionModel : public QAbstractListModel
// {
//     Q_OBJECT
//     QML_ELEMENT
//     /**
//      * @brief The current type of completion being done on the entered text.
//      *
//      * @sa AutoCompletionType
//      */
//     Q_PROPERTY(AutoCompletionType autoCompletionType READ autoCompletionType NOTIFY autoCompletionTypeChanged)
//
//     /**
//      * @brief The RoomListModel to be used for room completions.
//      */
//     Q_PROPERTY(RoomListModel *roomListModel READ roomListModel WRITE setRoomListModel NOTIFY roomListModelChanged)
//     RoomListModel *roomListModel() const;
//     void setRoomListModel(RoomListModel *roomListModel);
//
//     AutoCompletionType autoCompletionType() const;
//     void setAutoCompletionType(AutoCompletionType autoCompletionType);
//

// Q_SIGNALS:
//     void autoCompletionTypeChanged();
//     void userListModelChanged();

// private:
//     CompletionProxyModel *m_filterModel;
// };
