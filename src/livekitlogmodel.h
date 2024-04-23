// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QList>

#include "ffi.qpb.h"

class LivekitLogModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    static LivekitLogModel &instance() {
        static LivekitLogModel _instance;
        return _instance;
    }

    static LivekitLogModel *create(QQmlEngine *, QJSEngine *) {
        QQmlEngine::setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        MessageRole = Qt::DisplayRole,
    };

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;

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

    void addMessages(livekit::proto::LogRecordRepeated messages);

private:
    livekit::proto::LogRecordRepeated m_messages;
    LivekitLogModel() = default;
};
