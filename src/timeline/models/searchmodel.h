// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>

#include <Quotient/csapi/search.h>

#include "messagemodel.h"

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
class SearchModel : public MessageModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The text to search messages for.
     */
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

    /**
     * @brief Whether the model is currently searching for messages.
     */
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)

    /**
     * @brief If set, limits the search to this specific sender.
     */
    Q_PROPERTY(QString senderId READ senderId WRITE setSenderId NOTIFY senderIdChanged)

public:
    explicit SearchModel(QObject *parent = nullptr);

    QString searchText() const;
    void setSearchText(const QString &searchText);

    bool searching() const;

    QString senderId() const;
    void setSenderId(const QString &sender);

    /**
     * @brief Number of rows in the model.
     *
     * @sa QAbstractItemModel::rowCount
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Start searching for messages.
     */
    Q_INVOKABLE void search();

Q_SIGNALS:
    void searchTextChanged();
    void roomChanged();
    void searchingChanged();
    void senderIdChanged();

private:
    std::optional<std::reference_wrapper<const Quotient::RoomEvent>> getEventForIndex(QModelIndex index) const override;

    void setSearching(bool searching);

    QString m_searchText;
    std::optional<Quotient::SearchJob::ResultRoomEvents> m_result = std::nullopt;
    Quotient::SearchJob *m_job = nullptr;
    bool m_searching = false;
    QString m_senderId;
};
