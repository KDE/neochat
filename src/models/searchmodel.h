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

class SearchModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(Quotient::Connection *connection READ connection WRITE setConnection NOTIFY connectionChanged)
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)

public:
    enum Roles {
        DisplayRole = Qt::DisplayRole,
        EventTypeRole,
        ShowAuthorRole,
        AuthorRole,
        ShowSectionRole,
        SectionRole,
        TimeRole,
    };
    Q_ENUM(Roles);
    SearchModel(QObject *parent = nullptr);

    QString searchText() const;
    void setSearchText(const QString &searchText);

    Quotient::Connection *connection() const;
    void setConnection(Quotient::Connection *connection);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    Q_INVOKABLE void search();

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool searching() const;

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
