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

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        ComponentTypeRole = Qt::UserRole, /**< The type of component to visualise the message. */
        BlockRole, /**< The Blocks::Block for the delegate. */
        ReplyContentModelRole, /**< The MessageContentModel for the reply event. */
        ThreadRootRole, /**< The thread root event ID for the event. */
        EditableRole, /**< Whether the component can be edited. */
        CurrentFocusRole, /**< Whether the delegate should have focus. */
    };
    Q_ENUM(Roles)

    explicit MessageContentModel(QObject *parent = nullptr);

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
     * @brief Toggle spoiler for the component at the given row.
     */
    Q_INVOKABLE void toggleSpoiler(QModelIndex index);

Q_SIGNALS:
    /**
     * @brief Emit whenever new components are added.
     */
    void componentsUpdated();

protected:
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
    void updateSpoilers();
    void updateSpoiler(const QModelIndex &index);
};
