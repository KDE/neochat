// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QQmlEngine>
#include <QSortFilterProxyModel>

/**
 * @class StateFilterModel
 *
 * This class creates a custom QSortFilterProxyModel for filtering a list of state events.
 * Event types can be filtered out by adding them to m_stateEventTypesFiltered.
 */
class StateFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /**
     * @brief Custom filter function checking if an event type has been filtered out.
     *
     * The filter rejects a row if the state event type has been added to m_stateEventTypesFiltered.
     *
     * @sa m_stateEventTypesFiltered
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    /**
     * @brief Add an event type to m_stateEventTypesFiltered.
     *
     * @sa m_stateEventTypesFiltered
     */
    Q_INVOKABLE void addStateEventTypeFiltered(const QString &stateEventType);

    /**
     * @brief Remove an event type from m_stateEventTypesFiltered.
     *
     * @sa m_stateEventTypesFiltered
     */
    Q_INVOKABLE void removeStateEventTypeFiltered(const QString &stateEventType);

private:
    QStringList m_stateEventTypesFiltered;
};
