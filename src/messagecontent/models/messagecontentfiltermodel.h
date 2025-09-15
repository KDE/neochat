// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

/**
 * @class MessageContentFilterModel
 *
 * This model filters a message's contents.
 */
class MessageContentFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief Whether the author component should be shown.
     */
    Q_PROPERTY(bool showAuthor READ showAuthor WRITE setShowAuthor NOTIFY showAuthorChanged)

public:
    explicit MessageContentFilterModel(QObject *parent = nullptr);

    bool showAuthor() const;
    void setShowAuthor(bool showAuthor);

Q_SIGNALS:
    void showAuthorChanged();

protected:
    /**
     * @brief Whether a row should be shown out or not.
     *
     * @sa QSortFilterProxyModel::filterAcceptsRow
     */
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    bool m_showAuthor = true;
};
