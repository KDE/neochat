// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>

#include <KLocalizedString>

/**
 * @class PowerLevel
 *
 * This class is designed to define the PowerLevel enumeration.
 */
class PowerLevel : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    /**
     * @brief The type of delegate that is needed for the event.
     *
     * @note While similar this is not the matrix event or message type. This is
     *       to tell a QML ListView what delegate to show for each event. So while
     *       similar to the spec it is not the same.
     */
    enum Level {
        Member, /**< A basic member. */
        Moderator, /**< A moderator with enhanced powers. */
        Admin, /**< Power level 100. */
        Owner, /**< Power level 150. */
        Mute, /**< The level to remove posting privileges. */
        NUMLevels,
        Custom, /**< A non-standard value. Intentionally after NUMLevels so it doesn't appear in the model. */
        Creator, /**< The user creating the (co-)creating the room. */
    };
    Q_ENUM(Level);

    /**
     * @brief Return a string representation of the enum value.
     */
    static QString nameForLevel(Level level);

    /**
     * @brief Return the integer representation of the enum value.
     */
    static int valueForLevel(Level level);

    /**
     * @brief Return the enum value for the given integer power level.
     */
    static Level levelForValue(int value);
};

/**
 * @class PowerLevelModel
 *
 * A model visualize the allowed power levels.
 */
class PowerLevelModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool showMute READ showMute WRITE setShowMute NOTIFY showMuteChanged)

public:
    /**
     * @brief Defines the model roles.
     */
    enum Roles {
        NameRole = Qt::DisplayRole, /**< The power level name. */
        ValueRole = Qt::UserRole, /**< The power level value. */
    };
    Q_ENUM(Roles)

    explicit PowerLevelModel(QObject *parent = nullptr);

    [[nodiscard]] bool showMute() const;
    void setShowMute(bool showMute);

    /**
     * @brief Get the given role value at the given index.
     *
     * @sa QAbstractItemModel::data
     */
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Number of rows in the model.
     *
     * @sa  QAbstractItemModel::rowCount
     */
    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief Returns a mapping from Role enum values to role names.
     *
     * @sa Roles, QAbstractItemModel::roleNames()
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void showMuteChanged();

private:
    bool m_showMute = true;
};
