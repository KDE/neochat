// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QQmlIncubator>
#include <QQuickItem>
#include <QTimer>

#include "neochatroommember.h"
#include "timelinedelegate.h"

/**
 * @brief Incubator for creating instances of components as required.
 */
class MessageObjectIncubator : public QQmlIncubator
{
public:
    MessageObjectIncubator(std::function<void(QQuickItem *)> initialCallback,
                           std::function<void(MessageObjectIncubator *)> completedCallback,
                           std::function<void(MessageObjectIncubator *)> errorCallback);

private:
    void setInitialState(QObject *object) override;
    std::function<void(QQuickItem *)> m_initialCallback;
    void statusChanged(QQmlIncubator::Status status) override;
    std::function<void(MessageObjectIncubator *)> m_completedCallback;
    std::function<void(MessageObjectIncubator *)> m_errorCallback;
};

/**
 * @brief Delegate Item for all messages in the timeline.
 */
class MessageDelegateBase : public TimelineDelegate
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The message author.
     *
     * @sa NeochatRoomMember
     */
    Q_PROPERTY(NeochatRoomMember *author READ author WRITE setAuthor NOTIFY authorChanged FINAL REQUIRED)

    /**
     * @brief Whether the message is threaded.
     */
    Q_PROPERTY(bool isThreaded READ isThreaded WRITE setIsThreaded NOTIFY isThreadedChanged FINAL REQUIRED)

    /**
     * @brief The maximum width available to the content item.
     */
    Q_PROPERTY(qreal maxContentWidth READ maxContentWidth NOTIFY maxContentWidthChanged FINAL)

    /**
     * @brief The component to use to visualize a user avatar.
     */
    Q_PROPERTY(QQmlComponent *avatarComponent READ avatarComponent WRITE setAvatarComponent NOTIFY avatarComponentChanged FINAL)

    /**
     * @brief Whether the user avatar should be shown.
     *
     * @note An avatar is only shown if enabled.
     */
    Q_PROPERTY(bool showAuthor READ showAuthor WRITE setShowAuthor NOTIFY showAuthorChanged FINAL REQUIRED)

    /**
     * @brief Whether user avatars are enabled.
     */
    Q_PROPERTY(bool enableAvatars READ enableAvatars WRITE setEnableAvatars NOTIFY enableAvatarsChanged FINAL)

    /**
     * @brief The component to use to visualize a section.
     */
    Q_PROPERTY(QQmlComponent *sectionComponent READ sectionComponent WRITE setSectionComponent NOTIFY sectionComponentChanged FINAL)

    /**
     * @brief Whether to show the section component.
     */
    Q_PROPERTY(bool showSection READ showSection WRITE setShowSection NOTIFY showSectionChanged FINAL REQUIRED)

    /**
     * @brief The component to use to visualize other user read markers.
     */
    Q_PROPERTY(QQmlComponent *readMarkerComponent READ readMarkerComponent WRITE setReadMarkerComponent NOTIFY readMarkerComponentChanged FINAL)

    /**
     * @brief Whether to show the other user read markers.
     */
    Q_PROPERTY(bool showReadMarkers READ showReadMarkers WRITE setShowReadMarkers NOTIFY showReadMarkersChanged FINAL REQUIRED)

    /**
     * @brief The component to use to visualize the hover state in compact mode.
     */
    Q_PROPERTY(QQmlComponent *compactBackgroundComponent READ compactBackgroundComponent WRITE setCompactBackgroundComponentt NOTIFY
                   compactBackgroundComponentChanged FINAL)

    /**
     * @brief The component to use to visualize quick actions.
     */
    Q_PROPERTY(QQmlComponent *quickActionComponent READ quickActionComponent WRITE setQuickActionComponent NOTIFY quickActionComponentChanged FINAL)

    /**
     * @brief The component to use to visualize message selection.
     */
    Q_PROPERTY(QQmlComponent *selectionComponent READ selectionComponent WRITE setSelectionComponent NOTIFY selectionComponentChanged FINAL)

    /**
     * @brief Whether to show the selection component.
     */
    Q_PROPERTY(bool showSelection READ showSelection WRITE setShowSelection NOTIFY showSelectionChanged FINAL REQUIRED)

    /**
     * @brief Whether to use the compact mode appearance.
     */
    Q_PROPERTY(bool compactMode READ compactMode WRITE setCompactMode NOTIFY compactModeChanged FINAL)

    /**
     * @brief Whether to show messages by the local user on the right in bubble mode.
     */
    Q_PROPERTY(bool showLocalMessagesOnRight READ showLocalMessagesOnRight WRITE setShowLocalMessagesOnRight NOTIFY showLocalMessagesOnRightChanged FINAL)

    /**
     * @brief Whether the message should be highlighted temporarily.
     *
     * Normally triggered when jumping to the event in the timeline, e.g. when a reply
     * is clicked.
     */
    Q_PROPERTY(bool isTemporaryHighlighted READ isTemporaryHighlighted WRITE setIsTemporaryHighlighted NOTIFY isTemporaryHighlightedChanged FINAL)

    /**
     * @brief Whether the delegate is hovered.
     */
    Q_PROPERTY(bool hovered READ hovered NOTIFY hoveredChanged FINAL)

public:
    MessageDelegateBase(QQuickItem *parent = nullptr);
    ~MessageDelegateBase();

    NeochatRoomMember *author() const;
    void setAuthor(NeochatRoomMember *author);

    bool isThreaded() const;
    void setIsThreaded(bool isThreaded);

    qreal maxContentWidth() const;

    QQmlComponent *avatarComponent() const;
    void setAvatarComponent(QQmlComponent *avatarComponent);
    bool showAuthor() const;
    void setShowAuthor(bool showAuthor);
    bool enableAvatars() const;
    void setEnableAvatars(bool enableAvatars);

    QQmlComponent *sectionComponent() const;
    void setSectionComponent(QQmlComponent *sectionComponent);
    bool showSection() const;
    void setShowSection(bool showSection);

    QQmlComponent *readMarkerComponent() const;
    void setReadMarkerComponent(QQmlComponent *readMarkerComponent);
    bool showReadMarkers() const;
    void setShowReadMarkers(bool showReadMarkers);

    QQmlComponent *compactBackgroundComponent() const;
    void setCompactBackgroundComponentt(QQmlComponent *compactBackgroundComponent);
    bool compactMode() const;
    void setCompactMode(bool compactMode);

    QQmlComponent *quickActionComponent() const;
    void setQuickActionComponent(QQmlComponent *quickActionComponent);

    QQmlComponent *selectionComponent() const;
    void setSelectionComponent(QQmlComponent *selectionComponent);
    bool showSelection() const;
    void setShowSelection(bool showSelection);

    bool showLocalMessagesOnRight() const;
    void setShowLocalMessagesOnRight(bool showLocalMessagesOnRight);

    bool isTemporaryHighlighted() const;
    void setIsTemporaryHighlighted(bool isTemporaryHighlighted);

    bool hovered() const;

Q_SIGNALS:
    void authorChanged();
    void isThreadedChanged();
    void maxContentWidthChanged();
    void avatarComponentChanged();
    void showAuthorChanged();
    void enableAvatarsChanged();
    void sectionComponentChanged();
    void showSectionChanged();
    void readMarkerComponentChanged();
    void showReadMarkersChanged();
    void compactBackgroundComponentChanged();
    void quickActionComponentChanged();
    void selectionComponentChanged();
    void showSelectionChanged();
    void compactModeChanged();
    void showLocalMessagesOnRightChanged();
    void isTemporaryHighlightedChanged();
    void hoveredChanged();

private:
    DelegateSizeHelper m_contentSizeHelper;

    QPointer<NeochatRoomMember> m_author;

    bool m_isThreaded = false;

    std::vector<MessageObjectIncubator *> m_activeIncubators;

    QPointer<QQmlComponent> m_avatarComponent;
    bool m_avatarIncubating = false;
    QPointer<QQuickItem> m_avatarItem;
    bool m_showAuthor = false;
    bool m_enableAvatars = true;
    bool leaveAvatarSpace() const;
    bool showAvatar() const;
    void updateAvatar();

    QPointer<QQmlComponent> m_sectionComponent;
    bool m_sectionIncubating = false;
    QPointer<QQuickItem> m_sectionItem;
    bool m_showSection = false;
    void updateSection();

    QPointer<QQmlComponent> m_readMarkerComponent;
    bool m_readMarkerIncubating = false;
    QPointer<QQuickItem> m_readMarkerItem;
    bool m_showReadMarkers = false;
    void updateReadMarker();

    QPointer<QQmlComponent> m_compactBackgroundComponent;
    bool m_compactBackgroundIncubating = false;
    QPointer<QQuickItem> m_compactBackgroundItem;
    bool m_compactMode = false;
    void updateBackground();

    QPointer<QQmlComponent> m_quickActionComponent;
    bool m_quickActionIncubating = false;
    QPointer<QQuickItem> m_quickActionItem;

    QPointer<QQmlComponent> m_selectionComponent;
    bool m_selectionIncubating = false;
    QPointer<QQuickItem> m_selectionItem;
    bool m_showSelection = false;
    void updateSelection();

    bool m_showLocalMessagesOnRight = true;

    bool m_hovered = false;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;

    std::function<void(QQuickItem *)> m_objectInitialCallback = [this](QQuickItem *object) {
        if (!object) {
            return;
        }

        object->setParentItem(this);
        connect(object, &QQuickItem::implicitWidthChanged, this, &MessageDelegateBase::markAsDirty);
        connect(object, &QQuickItem::implicitHeightChanged, this, &MessageDelegateBase::markAsDirty);
        connect(object, &QQuickItem::visibleChanged, this, &MessageDelegateBase::markAsDirty);
    };
    std::function<void(MessageObjectIncubator *)> m_errorCallback = [this](MessageObjectIncubator *incubator) {
        if (!incubator) {
            return;
        }

        if (incubator->object()) {
            incubator->object()->deleteLater();
        }
        cleanupIncubator(incubator);
    };
    void cleanupIncubator(MessageObjectIncubator *incubator);
    void cleanupItem(QQuickItem *item);

    qreal m_spacing = 0.0;
    qreal m_avatarSize = 0.0;

    void setCurveValues() override;
    void setBaseRightPadding();
    void setPercentageValues(bool fillWidth = false);
    void setContentPadding();
    void updateImplicitHeight() override;
    bool showMessageOnRight() const;
    void resizeContent() override;

    QPointer<QTimer> m_temporaryHighlightTimer;

private Q_SLOTS:
    void updateQuickAction();
};
