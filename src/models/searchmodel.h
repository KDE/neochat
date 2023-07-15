// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QString>

#include <csapi/search.h>

namespace Quotient
{
class Connection;
}

class NeoChatRoom;

/**
 * @class SearchModel
 *
 * This class defines the model for visualising the results of a room message search.
 */
class SearchModel : public QAbstractListModel
{
    Q_OBJECT

    /**
     * @brief The text to search messages for.
     */
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

    /**
     * @brief The current connection that the model is using to search for messages.
     */
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)

    /**
     * @brief The current room that the search is being done from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)

    /**
     * @brief Whether the model is currently searching for messages.
     */
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)

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
        EventIdRole,
        ExcessReadMarkersRole,
        HighlightRole,
        ReadMarkersString,
        PlainTextRole,
        VerifiedRole,
        ReplyAuthorRole,
        ProgressInfoRole,
        IsReplyRole,
        ShowReactionsRole,
        ReplyRole,
        ReactionRole,
        ReplyMediaInfoRole,
        ReadMarkersRole,
        IsPendingRole,
        ShowReadMarkersRole,
        ReplyIdRole,
        MimeTypeRole,
        ShowLinkPreviewRole,
        LinkPreviewRole,
        SourceRole,
    };
    Q_ENUM(Roles)
    explicit SearchModel(QObject *parent = nullptr);

    QString searchText() const;
    void setSearchText(const QString &searchText);

    Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *connection);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    bool searching() const;

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Start searching for messages.
     */
    Q_INVOKABLE void search();

Q_SIGNALS:
    void searchTextChanged();
    void connectionChanged();
    void roomChanged();
    void searchingChanged();

private:
    void setSearching(bool searching);

    QString m_searchText;
    Quotient::Connection *m_connection = nullptr;
    NeoChatRoom *m_room = nullptr;
    Quotient::Omittable<Quotient::SearchJob::ResultRoomEvents> m_result = Quotient::none;
    Quotient::SearchJob *m_job = nullptr;
    bool m_searching = false;
};

QString renderDate(const QDateTime &dateTime);
