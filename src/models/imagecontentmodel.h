// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

/**
 * @class ImageContentModel
 *
 * This class defines the model for visualising a list of emojis.
 */
class ImageContentModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY categoryChanged)

public:
    ImageContentModel(QObject *parent = nullptr);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &idx, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa RoleNames, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] QString category() const;
    void setCategory(const QString &category);

    QVariant emojiData(int row, int role) const;
    QVariant accountData(int row, int role) const;
    QVariant roomData(int row, int role) const;

Q_SIGNALS:
    void categoryChanged();

private:
    QString m_category;
    QString m_roomId;
    QString m_stateKey;
};
