// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QSortFilterProxyModel>
#include <QQmlEngine>

class ImageContentSearchModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)

public:
    explicit ImageContentSearchModel(QObject *parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    [[nodiscard]] QString searchText() const;
    void setSearchText(QString searchText);

Q_SIGNALS:
    void searchTextChanged();

private:
    QString m_searchText;
};
