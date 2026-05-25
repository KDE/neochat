// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QImageReader>
#include <QQmlEngine>
#include <QTextDocumentFragment>

#ifndef Q_OS_ANDROID
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#endif

#include "block.h"
#include "enums/blocktype.h"
#include "fileinfo.h"
#include "filetype.h"
#include "linkpreviewer.h"
#include "models/reactionmodel.h"
#include "neochatroom.h"
#include "neochatroommember.h"

class NeoChatDateTime;

/**
 * @class MessageContentModel
 *
 * A model to visualise the content of a message.
 *
 * This is a base model designed to be extended. The inherited class needs to define
 * how the Blocks are added.
 */
class MessageContentModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief The room the chat bar is for.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief The author if the message.
     */
    Q_PROPERTY(NeochatRoomMember *author READ author NOTIFY authorChanged)
    Q_PROPERTY(QString eventId READ eventId CONSTANT)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        ComponentTypeRole = Qt::UserRole, /**< The type of component to visualise the message. */
        BlockRole, /**< The Blocks::Block for the delegate. */
        EventIdRole, /**< The matrix event ID of the event. */
        DateTimeRole, /**< The timestamp for when the event was sent (as a NeoChatDateTime). */
        AuthorRole, /**< The author of the event. */
        FileTransferInfoRole, /**< FileTransferInfo for any downloading files. */
        PollHandlerRole, /**< The PollHandler for the event, if any. */
        ReplyContentModelRole, /**< The MessageContentModel for the reply event. */
        ThreadRootRole, /**< The thread root event ID for the event. */
        EditableRole, /**< Whether the component can be edited. */
        CurrentFocusRole, /**< Whether the delegate should have focus. */
        MediaHiddenRole, /**< Whether the media should be visible or not. */
    };
    Q_ENUM(Roles)

    explicit MessageContentModel(QObject *parent = nullptr);
    explicit MessageContentModel(NeoChatRoom *room, const QString &eventId, MessageContentModel *parent = nullptr);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    static QHash<int, QByteArray> roleNamesStatic();

    /**
     * @brief The Matrix event ID of the message.
     */
    Q_INVOKABLE QString eventId() const;

    /**
     * @brief The author of the message.
     */
    Q_INVOKABLE NeochatRoomMember *author() const;

    /**
     * @brief Toggle spoiler for the component at the given row.
     */
    Q_INVOKABLE void toggleSpoiler(QModelIndex index);

    /**
     * @brief Hides the media contained in this message.
     */
    Q_INVOKABLE void hideMedia();

    /**
     * @brief Shows the media contained in this message.
     */
    Q_INVOKABLE void showMedia();

    /**
     * @brief If the media is hidden for this message.
     */
    Q_INVOKABLE bool isMediaHidden();

    static void setSetMediaHidden(std::function<void(const QString &, bool)> func);
    static void setMediaShouldBeHidden(std::function<bool(const QString &)> func);

Q_SIGNALS:
    void roomChanged(NeoChatRoom *oldRoom, NeoChatRoom *newRoom);
    void authorChanged();

    /**
     * @brief Emit whenever new components are added.
     */
    void componentsUpdated();

protected:
    QPointer<NeoChatRoom> m_room;
    QString m_eventId;

    /**
     * @brief NeoChatDateTime for the message.
     *
     * The default implementation returns the current time.
     */
    virtual NeoChatDateTime dateTime() const;

    /**
     * @brief The author of the message.
     *
     * The default implementation returns the local user.
     */
    virtual QString authorId() const;

    /**
     * @brief Thread root ID for the message if in a thread.
     *
     * The default implementation returns an empty string.
     */
    virtual QString threadRootId() const;

    Blocks::BlockPtrs m_components;
    bool hasComponentType(Blocks::Type type) const;
    bool hasComponentType(const QList<Blocks::Type> &types) const;
    void forEachComponentOfType(Blocks::Type type, std::function<Blocks::BlockPtrsIt(Blocks::BlockPtrsIt)> function);
    void forEachComponentOfType(QList<Blocks::Type> types, std::function<Blocks::BlockPtrsIt(Blocks::BlockPtrsIt)> function);

    QPointer<MessageContentModel> m_replyModel;

    bool m_editableActive = false;
    QPersistentModelIndex m_currentFocusComponent = {};

private:
    std::function<Blocks::BlockPtrsIt(const Blocks::BlockPtrsIt &)> m_fileInfoFunction = [this](Blocks::BlockPtrsIt it) {
        Q_EMIT dataChanged(index(it - m_components.begin()), index(it - m_components.begin()), {MessageContentModel::FileTransferInfoRole});
        return ++it;
    };

    void updateSpoilers();
    void updateSpoiler(const QModelIndex &index);

    bool m_mediaHidden = false;

    static std::function<void(const QString &, bool)> m_setMediaHidden;
    static std::function<bool(const QString &)> m_mediaShouldBeHidden;
};
