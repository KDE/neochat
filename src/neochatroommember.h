// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QPointer>
#include <qqmlintegration.h>

#include <Quotient/roommember.h>
#include <Quotient/uri.h>

class NeoChatRoom;

/**
 * @class NeochatRoomMember
 *
 * This class is a shim around RoomMember that can be safety passed to QML.
 *
 * Because RoomMember has an internal pointer to a RoomMemberEvent it is
 * designed to be created used then quickly discarded as the stste event is changed
 * every time the member updates. Passing these to QML which will hold onto them
 * can lead to accessing an already deleted Quotient::RoomMemberEvent relatively easily.
 *
 * This class instead holds a member ID and can therefore always safely create and
 * access Quotient::RoomMember objects while being used as long as needed by QML.
 *
 * @note This is only needed to pass to QML if only accessing in CPP RoomMmeber can
 *       be used safely.
 *
 * @note The interface is the same as Quotient::RoomMember.
 *
 * @sa Quotient::RoomMember
 */
class NeochatRoomMember : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(Quotient::Uri uri READ uri CONSTANT)
    Q_PROPERTY(bool isLocalMember READ isLocalMember CONSTANT)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameUpdated)
    Q_PROPERTY(QString htmlSafeDisplayName READ htmlSafeDisplayName NOTIFY displayNameUpdated)
    Q_PROPERTY(QString fullName READ fullName NOTIFY displayNameUpdated)
    Q_PROPERTY(QString htmlSafeFullName READ htmlSafeFullName NOTIFY displayNameUpdated)
    Q_PROPERTY(QString disambiguatedName READ disambiguatedName NOTIFY displayNameUpdated)
    Q_PROPERTY(QString htmlSafeDisambiguatedName READ htmlSafeDisambiguatedName NOTIFY displayNameUpdated)
    Q_PROPERTY(int hue READ hue CONSTANT)
    Q_PROPERTY(qreal hueF READ hueF CONSTANT)
    Q_PROPERTY(QColor color READ color CONSTANT)
    Q_PROPERTY(QUrl avatarUrl READ avatarUrl NOTIFY avatarUpdated)

public:
    NeochatRoomMember() = default;
    explicit NeochatRoomMember(NeoChatRoom *room, const QString &memberId);

    QString id() const;
    Quotient::Uri uri() const;
    bool isLocalMember() const;
    Quotient::Membership membershipState() const;
    QString name() const;
    QString displayName() const;
    QString htmlSafeDisplayName() const;
    QString fullName() const;
    QString htmlSafeFullName() const;
    QString disambiguatedName() const;
    QString htmlSafeDisambiguatedName() const;
    int hue() const;
    qreal hueF() const;
    QColor color() const;
    QString avatarMediaId() const;
    QUrl avatarUrl() const;

Q_SIGNALS:
    void displayNameUpdated();
    void avatarUpdated();

private:
    QPointer<NeoChatRoom> m_room;
    const QString m_memberId = QString();
};
