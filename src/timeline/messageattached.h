// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QQmlEngine>
#include <QQuickAttachedPropertyPropagator>
#include <QQuickItem>

#include "models/messagecontentmodel.h"
#include "neochatroom.h"

class MessageAttached : public QQuickAttachedPropertyPropagator
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Message)
    QML_ATTACHED(MessageAttached)
    QML_UNCREATABLE("")

    /**
     * @brief The room that the message comes from.
     */
    Q_PROPERTY(NeoChatRoom *room READ room WRITE setRoom NOTIFY roomChanged FINAL)

    /**
     * @brief The timeline for the current message.
     */
    Q_PROPERTY(QQuickItem *timeline READ timeline WRITE setTimeline NOTIFY timelineChanged FINAL)

    /**
     * @brief The content model for the current message.
     */
    Q_PROPERTY(MessageContentModel *contentModel READ contentModel WRITE setContentModel NOTIFY contentModelChanged FINAL)

    /**
     * @brief The index of the message in the timeline
     */
    Q_PROPERTY(int index READ index WRITE setIndex NOTIFY indexChanged FINAL)

    /**
     * @brief The width available to the message content.
     */
    Q_PROPERTY(qreal maxContentWidth READ maxContentWidth WRITE setMaxContentWidth NOTIFY maxContentWidthChanged FINAL)

    /**
     * @brief The current selected message text.
     */
    Q_PROPERTY(QString selectedText READ selectedText WRITE setSelectedText NOTIFY selectedTextChanged FINAL)

    /**
     * @brief The text of a hovered link if any.
     */
    Q_PROPERTY(QString hoveredLink READ hoveredLink WRITE setHoveredLink NOTIFY hoveredLinkChanged FINAL)

public:
    explicit MessageAttached(QObject *parent = nullptr);

    static MessageAttached *qmlAttachedProperties(QObject *object);

    NeoChatRoom *room() const;
    void setRoom(NeoChatRoom *room);

    QQuickItem *timeline() const;
    void setTimeline(QQuickItem *timeline);

    MessageContentModel *contentModel() const;
    void setContentModel(MessageContentModel *contentModel);

    int index() const;
    void setIndex(int index);

    qreal maxContentWidth() const;
    void setMaxContentWidth(qreal maxContentWidth);

    QString selectedText() const;
    void setSelectedText(const QString &selectedTest);

    QString hoveredLink() const;
    void setHoveredLink(const QString &hoveredLink);

Q_SIGNALS:
    void roomChanged();
    void timelineChanged();
    void contentModelChanged();
    void indexChanged();
    void maxContentWidthChanged();
    void selectedTextChanged();
    void hoveredLinkChanged();

protected:
    void propagateMessage(MessageAttached *message);
    void attachedParentChange(QQuickAttachedPropertyPropagator *newParent, QQuickAttachedPropertyPropagator *oldParent) override;

private:
    QPointer<NeoChatRoom> m_room;
    bool m_explicitRoom = false;

    QPointer<QQuickItem> m_timeline;
    bool m_explicitTimeline = false;

    QPointer<MessageContentModel> m_contentModel;
    bool m_explicitContentModel = false;

    int m_index = -1;
    bool m_explicitIndex = false;

    qreal m_maxContentWidth = -1;
    bool m_explicitMaxContentWidth = false;

    QString m_selectedText = {};
    bool m_explicitSelectedText = false;

    QString m_hoveredLink = {};
    bool m_explicitHoveredLink = false;
};
