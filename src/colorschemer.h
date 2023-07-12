// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>

class QAbstractItemModel;
class KColorSchemeManager;

/**
 * @class ColorSchemer
 *
 * A class to provide a wrapper around KColorSchemeManager to make it available in
 * QML.
 *
 * @sa KColorSchemeManager
 */
class ColorSchemer : public QObject
{
    Q_OBJECT

    /**
     * @brief A QAbstractItemModel of all available color schemes.
     *
     * @sa QAbstractItemModel
     */
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)

public:
    explicit ColorSchemer(QObject *parent = nullptr);
    ~ColorSchemer();

    QAbstractItemModel *model() const;

    /**
     * @brief Activates the KColorScheme identified by the provided index.
     *
     * @sa KColorScheme
     */
    Q_INVOKABLE void apply(int idx);

    /**
     * @brief Activates the KColorScheme with the given name.
     *
     * @sa KColorScheme
     */
    Q_INVOKABLE void apply(const QString &name);

    /**
     * @brief Returns the index for the scheme with the given name.
     */
    Q_INVOKABLE int indexForScheme(const QString &name) const;

    /**
     * @brief Returns the name for the scheme with the given index.
     */
    Q_INVOKABLE QString nameForIndex(int index) const;

private:
    KColorSchemeManager *c;
};
