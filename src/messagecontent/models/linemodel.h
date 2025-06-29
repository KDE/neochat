// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QAbstractTextDocumentLayout>
#include <QQmlEngine>
#include <QQuickTextDocument>
#include <QTextBlock>
#include <qtmetamacros.h>

/**
 * @class LineModel
 *
 * A model to provide line info for a QQuickTextDocument.
 *
 * @sa QQuickTextDocument
 */
class LineModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The QQuickTextDocument that is being handled.
     */
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        LineHeightRole = Qt::UserRole + 1, /**< The delegate type of the message. */
    };
    Q_ENUM(Roles)

    explicit LineModel(QObject *parent = nullptr);

    [[nodiscard]] QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

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

    /**
     * @brief Reset the model.
     *
     * This needs to be called when the QQuickTextDocument container changes width
     * or height as this may change line heights due to wrapping.
     *
     * @sa QQuickTextDocument
     */
    Q_INVOKABLE void resetModel();

Q_SIGNALS:
    void documentChanged();

private:
    QPointer<QQuickTextDocument> m_document = nullptr;
};
