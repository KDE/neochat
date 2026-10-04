// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <functional>
#include <qqmlintegration.h>

#include "events/imagepackevent.h"

class StickerList : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    /**
     * @brief Return the name of the list.
     */
    Q_PROPERTY(QString name READ name CONSTANT)

    /**
     * @brief Return the source URL for list avatar.
     */
    Q_PROPERTY(QUrl source READ source CONSTANT)

public:
    explicit StickerList(const Quotient::ImagePackEventContent &imagePackContent, std::function<QUrl(QUrl)> makeMediaUrlFunction, QObject *parent = nullptr);

    QString name() const;
    QUrl source() const;

    /**
     * @brief Return the number of stickers in the list.
     */
    qsizetype size() const;

    /**
     * @brief Return the Completion at the given index.
     *
     * std::nullopt if i is not a valid index.
     *
     * @sa Completion
     */
    QVariant data(qsizetype row, int role = Qt::DisplayRole) const;

Q_SIGNALS:
    void stickersAdded(StickerList *list, qsizetype first, qsizetype last);

    void stickersRemoved(StickerList *list, qsizetype first, qsizetype last);

private:
    Quotient::ImagePackEventContent m_imagePackContent;

    std::function<QUrl(QUrl)> m_makeMediaUrlFunction;
};
