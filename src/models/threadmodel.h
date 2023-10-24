// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/csapi/relations.h>
#include <Quotient/events/roommessageevent.h>
#include <qpointer.h>

#include "linkpreviewer.h"

class NeoChatRoom;
class ReactionModel;

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
     * @brief The Matrix ID for the root mess
     */
    Q_PROPERTY(QString threadRootId READ threadRootId CONSTANT)

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
        PlainTextRole,
        GenericDisplayRole,
        EventIdRole,
        DelegateTypeRole,
        AuthorRole,
        ShowAuthorRole,
        TimeRole,
        TimeStringRole,
        SectionRole,
        ShowSectionRole,
        HighlightRole,
        SpecialMarksRole,
        MediaInfoRole,
        LinkPreviewRole,
        ShowLinkPreviewRole,
        ReactionRole,
        ShowReactionsRole,
        IsReplyRole,
        ReplyIdRole,
        ReplyDelegateTypeRole,
        ReplyAuthorRole,
        ReplyDisplayRole,
        ReplyMediaInfoRole,
        IsThreadedRole,
        ThreadRootRole,
        LatitudeRole,
        LongitudeRole,
        AssetRole,
        ReadMarkersRole,
        ExcessReadMarkersRole,
        ReadMarkersString,
        ShowReadMarkersRole,
        ProgressInfoRole,
        VerifiedRole,
        IsPendingRole,
    };
    Q_ENUM(Roles)

    explicit ThreadModel(const NeoChatRoom *room, const QString &threadRootId, QObject *parent = nullptr);

    QString threadRootId() const;

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

    /**
     * @brief Whether there is more data available for the model to fetch.
     *
     * @sa QAbstractItemModel::canFetchMore()
     */
    bool canFetchMore(const QModelIndex &parent) const override;

    /**
     * @brief Fetches the next batch of model data if any is available.
     *
     * @sa QAbstractItemModel::fetchMore()
     */
    void fetchMore(const QModelIndex &parent) override;

Q_SIGNALS:
    void loadingChanged();

private:
    const NeoChatRoom *m_room;
    const QString m_threadRootId;

    Quotient::RoomEvents m_events = {};
    QMap<QString, QSharedPointer<LinkPreviewer>> m_linkPreviewers;
    QMap<QString, QSharedPointer<ReactionModel>> m_reactionModels;

    QPointer<Quotient::GetRelatingEventsWithRelTypeJob> m_currentJob = nullptr;
    Quotient::Omittable<QString> m_nextBatch = QString();
    bool m_loading = false;

    void intializeModel();

    void createEventObjects(const Quotient::RoomMessageEvent *event);
};
Q_DECLARE_METATYPE(ThreadModel *)
