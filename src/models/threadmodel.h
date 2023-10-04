// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/events/roomevent.h>
#include <Quotient/events/roommessageevent.h>

class NeoChatRoom;

/**
 * @class ThreadModel
 *
 * This class defines the model for visualising a thread.
 */
class ThreadModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief Whether the model is currently loading messages.
     */
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)

public:
    /**
     * @brief Defines the model roles.
     *
     * For documentation of the roles, see MessageEventModel.
     *
     * Some of the roles exist only for compatibility with the MessageEventModel,
     * since the same delegates are used.
     */
    enum Roles {
        DisplayRole = Qt::DisplayRole,
        DelegateTypeRole,
        ShowAuthorRole,
        AuthorRole,
        ShowSectionRole,
        SectionRole,
        TimeRole,
        TimeStringRole,
        EventIdRole,
        ExcessReadMarkersRole,
        HighlightRole,
        ReadMarkersString,
        PlainTextRole,
        VerifiedRole,
        ProgressInfoRole,
        ShowReactionsRole,
        IsReplyRole,
        ReplyAuthorRole,
        ReplyIdRole,
        ReplyDelegateTypeRole,
        ReplyDisplayRole,
        ReplyMediaInfoRole,
        ReactionRole,
        ReadMarkersRole,
        IsPendingRole,
        ShowReadMarkersRole,
        MimeTypeRole,
        ShowLinkPreviewRole,
        LinkPreviewRole,
    };
    Q_ENUM(Roles)

    explicit ThreadModel(const NeoChatRoom *room, const QString &threadRootId, QObject *parent = nullptr);

    bool loading() const;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void loadingChanged();

private:
    const NeoChatRoom *m_room;
    const QString m_threadRootId;

    Quotient::RoomEvents m_events;

    bool m_loading = false;

    void intializeModel();
};
Q_DECLARE_METATYPE(ThreadModel *)
