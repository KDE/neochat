// SPDX-FileCopyrightText: 2025 Arno Rehn <arno@arnorehn.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef WIDGETMODEL_H
#define WIDGETMODEL_H

#include <QAbstractListModel>
#include <QQmlEngine>

#include "neochatroom.h"

class WidgetModelPrivate;

/**
 * @class WidgetModel
 *
 * `WidgetModel` provides a list model of widgets for a given room.
 *
 * It also supports adding and removing widgets.
 */
class WidgetModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The room for which this model lists the widgets
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged)
    /**
     * @brief The row index of the first Jitsi widget
     */
    Q_PROPERTY(int jitsiIndex READ jitsiIndex NOTIFY jitsiIndexChanged)
public:
    enum Roles {
        TextRole = Qt::DisplayRole,
        UrlRole = Qt::UserRole,
        TypeRole,
    };
    Q_ENUM(Roles)

    explicit WidgetModel(QObject *parent = nullptr);
    ~WidgetModel() override;

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *newRoom);

    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int roleName) const override;
    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;

    int jitsiIndex() const;

    /**
     * @brief Adds a new Jitsi widget
     * @return The URL of the newly added Jitsi conference
     */
    Q_INVOKABLE QUrl addJitsiConference();
    /**
     * @brief Removes the widget at @p index
     * @return `true` on success, `false` otherwise
     */
    Q_INVOKABLE bool removeWidget(int index);

Q_SIGNALS:
    void roomChanged();
    void jitsiIndexChanged();

private:
    Q_DECLARE_PRIVATE(WidgetModel)
    const std::unique_ptr<WidgetModelPrivate> d_ptr;
};

#endif // WIDGETMODEL_H
