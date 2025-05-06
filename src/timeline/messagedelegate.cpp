// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagedelegate.h"
#include "timelinedelegate.h"

#include <algorithm>
#include <cmath>

MessageObjectIncubator::MessageObjectIncubator(std::function<void(QQuickItem *)> initialCallback,
                                               std::function<void(MessageObjectIncubator *)> completedCallback,
                                               std::function<void(MessageObjectIncubator *)> errorCallback)
    : QQmlIncubator(QQmlIncubator::Asynchronous)
    , m_initialCallback(initialCallback)
    , m_completedCallback(completedCallback)
    , m_errorCallback(errorCallback)
{
}

void MessageObjectIncubator::setInitialState(QObject *object)
{
    auto item = qobject_cast<QQuickItem *>(object);
    if (item) {
        m_initialCallback(item);
    }
}

void MessageObjectIncubator::statusChanged(QQmlIncubator::Status status)
{
    if (status == QQmlIncubator::Error && m_errorCallback) {
        m_errorCallback(this);
    }
    if (status == QQmlIncubator::Ready && m_completedCallback) {
        m_completedCallback(this);
    }
}

MessageDelegateBase::MessageDelegateBase(QQuickItem *parent)
    : TimelineDelegate(parent)
{
    m_contentSizeHelper.setParentItem(this);
    setPercentageValues();

    connect(this, &MessageDelegateBase::leftPaddingChanged, this, &MessageDelegateBase::setContentPadding);
    connect(this, &MessageDelegateBase::rightPaddingChanged, this, &MessageDelegateBase::setContentPadding);
    connect(&m_contentSizeHelper, &DelegateSizeHelper::availableWidthChanged, this, &MessageDelegateBase::setContentPadding);
    connect(&m_contentSizeHelper, &DelegateSizeHelper::availableWidthChanged, this, &MessageDelegateBase::maxContentWidthChanged);
    connect(&m_contentSizeHelper, &DelegateSizeHelper::availableWidthChanged, this, &MessageDelegateBase::markAsDirty);
}

NeochatRoomMember *MessageDelegateBase::author() const
{
    return m_author;
}

void MessageDelegateBase::setAuthor(NeochatRoomMember *author)
{
    if (author == m_author) {
        return;
    }
    m_author = author;
    Q_EMIT authorChanged();

    setContentPadding();
    markAsDirty();
}

bool MessageDelegateBase::isThreaded() const
{
    return m_isThreaded;
}

void MessageDelegateBase::setIsThreaded(bool isThreaded)
{
    if (isThreaded == m_isThreaded) {
        return;
    }
    m_isThreaded = isThreaded;
    setAlwaysFillWidth(m_isThreaded || m_compactMode);
    setPercentageValues(m_isThreaded || m_compactMode);
    Q_EMIT isThreadedChanged();
}

void MessageDelegateBase::setCurveValues()
{
    m_spacing = qreal(m_units->smallSpacing());
    m_avatarSize = qreal(m_units->gridUnit() + m_units->largeSpacing() * 2);

    m_sizeHelper.setLeftPadding(qreal(m_units->largeSpacing()));
    setBaseRightPadding();

    m_sizeHelper.setStartBreakpoint(qreal(m_units->gridUnit() * 46));
    m_sizeHelper.setEndBreakpoint(qreal(m_units->gridUnit() * 66));
    m_sizeHelper.setMaxWidth(qreal(m_units->gridUnit() * 60));

    m_contentSizeHelper.setStartBreakpoint(qreal(m_units->gridUnit() * 25));
    m_contentSizeHelper.setEndBreakpoint(qreal(m_units->gridUnit() * 40));
    m_contentSizeHelper.setMaxWidth(qreal(m_units->gridUnit() * 60));

    setContentPadding();
}

void MessageDelegateBase::setBaseRightPadding()
{
    if (!m_units) {
        return;
    }

    if (m_compactMode && width() > m_units->gridUnit() * 30) {
        m_sizeHelper.setRightPadding(qreal(m_units->gridUnit() * 2 + m_units->largeSpacing()));
    } else {
        m_sizeHelper.setRightPadding(qreal(m_units->largeSpacing()));
    }
}

void MessageDelegateBase::setPercentageValues(bool fillWidth)
{
    if (fillWidth) {
        m_contentSizeHelper.setStartPercentWidth(100);
        m_contentSizeHelper.setEndPercentWidth(100);
    } else {
        m_contentSizeHelper.setStartPercentWidth(90);
        m_contentSizeHelper.setEndPercentWidth(60);
    }
}

void MessageDelegateBase::setContentPadding()
{
    m_contentSizeHelper.setLeftPadding(m_sizeHelper.leftX() + (leaveAvatarSpace() ? m_avatarSize + m_spacing : 0));
    m_contentSizeHelper.setRightPadding(m_sizeHelper.rightPadding());
}

qreal MessageDelegateBase::maxContentWidth() const
{
    return m_isThreaded || m_alwaysFillWidth ? m_contentSizeHelper.maxAvailableWidth() : m_contentSizeHelper.availableWidth();
}

void MessageDelegateBase::cleanupIncubator(MessageObjectIncubator *incubator)
{
    incubator->clear();
    delete incubator;
}

void MessageDelegateBase::cleanupItem(QQuickItem *item)
{
    if (!item) {
        return;
    }
    item->setParentItem(nullptr);
    item->disconnect(this);
    item->deleteLater();
}

QQmlComponent *MessageDelegateBase::avatarComponent() const
{
    return m_avatarComponent;
}

void MessageDelegateBase::setAvatarComponent(QQmlComponent *avatarComponent)
{
    if (avatarComponent == m_avatarComponent) {
        return;
    }
    m_avatarComponent = avatarComponent;
    Q_EMIT avatarComponentChanged();

    updateAvatar();
}

bool MessageDelegateBase::showAuthor() const
{
    return m_showAuthor;
}

void MessageDelegateBase::setShowAuthor(bool showAuthor)
{
    if (showAuthor == m_showAuthor) {
        return;
    }
    m_showAuthor = showAuthor;
    Q_EMIT showAuthorChanged();

    updateAvatar();
}

bool MessageDelegateBase::enableAvatars() const
{
    return m_enableAvatars;
}

void MessageDelegateBase::setEnableAvatars(bool enableAvatars)
{
    if (enableAvatars == m_enableAvatars) {
        return;
    }
    m_enableAvatars = enableAvatars;
    Q_EMIT enableAvatarsChanged();

    updateAvatar();
}

bool MessageDelegateBase::leaveAvatarSpace() const
{
    return m_enableAvatars && !showMessageOnRight();
}

bool MessageDelegateBase::showAvatar() const
{
    return m_enableAvatars && m_showAuthor && !showMessageOnRight();
}

void MessageDelegateBase::updateAvatar()
{
    if (m_avatarComponent && showAvatar() && !m_avatarItem && !m_avatarIncubating) {
        const auto avatarIncubator = new MessageObjectIncubator(
            m_objectInitialCallback,
            [this](MessageObjectIncubator *incubator) {
                if (!incubator) {
                    return;
                }
                const auto avatarObject = qobject_cast<QQuickItem *>(incubator->object());
                if (avatarObject) {
                    // The setting may have changed during the incubation period.
                    if (showAvatar()) {
                        m_avatarItem = avatarObject;
                    } else {
                        cleanupItem(avatarObject);
                    }
                    markAsDirty();
                }
                m_avatarIncubating = false;
                cleanupIncubator(incubator);
            },
            m_errorCallback);
        m_avatarComponent->create(*avatarIncubator, qmlContext(m_avatarComponent));
        m_avatarIncubating = true;
    } else if (!showAvatar() && m_avatarItem) {
        cleanupItem(m_avatarItem);
        markAsDirty();
    }
}

QQmlComponent *MessageDelegateBase::sectionComponent() const
{
    return m_sectionComponent;
}

void MessageDelegateBase::setSectionComponent(QQmlComponent *sectionComponent)
{
    if (sectionComponent == m_sectionComponent) {
        return;
    }
    m_sectionComponent = sectionComponent;
    Q_EMIT sectionComponentChanged();

    updateSection();
}

bool MessageDelegateBase::showSection() const
{
    return m_showSection;
}

void MessageDelegateBase::setShowSection(bool showSection)
{
    if (showSection == m_showSection) {
        return;
    }
    m_showSection = showSection;
    Q_EMIT showSectionChanged();

    updateSection();
}

void MessageDelegateBase::updateSection()
{
    if (m_sectionComponent && m_showSection && !m_sectionItem && !m_sectionIncubating) {
        const auto sectionIncubator = new MessageObjectIncubator(
            m_objectInitialCallback,
            [this](MessageObjectIncubator *incubator) {
                if (!incubator) {
                    return;
                }
                const auto sectionObject = qobject_cast<QQuickItem *>(incubator->object());
                if (sectionObject) {
                    // The setting may have changed during the incubation period.
                    if (m_showSection) {
                        m_sectionItem = sectionObject;
                    } else {
                        cleanupItem(sectionObject);
                    }
                    markAsDirty();
                }
                m_sectionIncubating = false;
                cleanupIncubator(incubator);
            },
            m_errorCallback);
        m_sectionComponent->create(*sectionIncubator, qmlContext(m_sectionComponent));
        m_sectionIncubating = true;
    } else if (!m_showSection && m_sectionItem) {
        cleanupItem(m_sectionItem);
        markAsDirty();
    }
}

QQmlComponent *MessageDelegateBase::readMarkerComponent() const
{
    return m_readMarkerComponent;
}

void MessageDelegateBase::setReadMarkerComponent(QQmlComponent *readMarkerComponent)
{
    if (readMarkerComponent == m_readMarkerComponent) {
        return;
    }
    m_readMarkerComponent = readMarkerComponent;
    Q_EMIT readMarkerComponentChanged();

    updateReadMarker();
}

bool MessageDelegateBase::showReadMarkers() const
{
    return m_showReadMarkers;
}

void MessageDelegateBase::setShowReadMarkers(bool showReadMarkers)
{
    if (showReadMarkers == m_showReadMarkers) {
        return;
    }
    m_showReadMarkers = showReadMarkers;
    Q_EMIT showReadMarkersChanged();

    updateReadMarker();
}

void MessageDelegateBase::updateReadMarker()
{
    if (m_readMarkerComponent && m_showReadMarkers && !m_readMarkerItem && !m_readMarkerIncubating) {
        const auto readMarkerIncubator = new MessageObjectIncubator(
            m_objectInitialCallback,
            [this](MessageObjectIncubator *incubator) {
                if (!incubator) {
                    return;
                }

                const auto readMarkerObject = qobject_cast<QQuickItem *>(incubator->object());
                if (readMarkerObject) {
                    if (m_showReadMarkers) {
                        m_readMarkerItem = readMarkerObject;
                    } else {
                        cleanupItem(readMarkerObject);
                    }
                    markAsDirty();
                }
                m_readMarkerIncubating = false;
                cleanupIncubator(incubator);
            },
            m_errorCallback);
        m_readMarkerComponent->create(*readMarkerIncubator, qmlContext(m_readMarkerComponent));
        m_readMarkerIncubating = true;
    } else if (!m_showReadMarkers && m_readMarkerItem) {
        cleanupItem(m_readMarkerItem);
        markAsDirty();
    }
}

QQmlComponent *MessageDelegateBase::compactBackgroundComponent() const
{
    return m_compactBackgroundComponent;
}

void MessageDelegateBase::setCompactBackgroundComponentt(QQmlComponent *compactBackgroundComponent)
{
    if (compactBackgroundComponent == m_compactBackgroundComponent) {
        return;
    }
    m_compactBackgroundComponent = compactBackgroundComponent;
    Q_EMIT compactBackgroundComponentChanged();

    updateBackground();
}

bool MessageDelegateBase::compactMode() const
{
    return m_compactMode;
}

void MessageDelegateBase::setCompactMode(bool compactMode)
{
    if (compactMode == m_compactMode) {
        return;
    }
    m_compactMode = compactMode;
    setAlwaysFillWidth(m_isThreaded || m_compactMode);
    setPercentageValues(m_isThreaded || m_compactMode);
    setAcceptHoverEvents(m_compactMode);
    setBaseRightPadding();

    Q_EMIT compactModeChanged();
    Q_EMIT maxContentWidthChanged();

    updateBackground();
}

void MessageDelegateBase::updateBackground()
{
    if (m_compactBackgroundComponent && m_compactMode && m_hovered && !m_compactBackgroundItem && !m_compactBackgroundIncubating) {
        const auto compactBackgroundIncubator = new MessageObjectIncubator(
            m_objectInitialCallback,
            [this](MessageObjectIncubator *incubator) {
                if (!incubator) {
                    return;
                }

                const auto compactBackgroundObject = qobject_cast<QQuickItem *>(incubator->object());
                if (compactBackgroundObject) {
                    if (m_compactMode) {
                        m_compactBackgroundItem = compactBackgroundObject;
                    } else {
                        cleanupItem(compactBackgroundObject);
                    }
                    markAsDirty();
                }
                cleanupIncubator(incubator);
                m_compactBackgroundIncubating = false;
            },
            m_errorCallback);
        m_compactBackgroundComponent->create(*compactBackgroundIncubator, qmlContext(m_compactBackgroundComponent));
        m_compactBackgroundIncubating = true;
    } else if (m_compactBackgroundItem && !m_hovered) {
        cleanupItem(m_compactBackgroundItem);
        markAsDirty();
    }
}

bool MessageDelegateBase::showLocalMessagesOnRight() const
{
    return m_showLocalMessagesOnRight;
}

void MessageDelegateBase::setShowLocalMessagesOnRight(bool showLocalMessagesOnRight)
{
    if (showLocalMessagesOnRight == m_showLocalMessagesOnRight) {
        return;
    }
    m_showLocalMessagesOnRight = showLocalMessagesOnRight;
    Q_EMIT showLocalMessagesOnRightChanged();

    setContentPadding();
    updateAvatar();
}

void MessageDelegateBase::updateImplicitHeight()
{
    qreal implicitHeight = 0.0;
    int numObj = 0;
    if (m_showSection && m_sectionItem) {
        implicitHeight += m_sectionItem->implicitHeight();
        numObj++;
    }
    qreal avatarHeight = 0.0;
    qreal contentHeight = 0.0;
    if (showAvatar() && m_avatarItem) {
        m_avatarItem->setImplicitWidth(m_avatarSize);
        m_avatarItem->setImplicitHeight(m_avatarSize);
        avatarHeight = m_avatarItem->implicitHeight();
    }
    if (m_contentItem) {
        contentHeight = m_contentItem->implicitHeight();
    }
    implicitHeight += std::max(avatarHeight, contentHeight);
    if (avatarHeight > 0 || contentHeight > 0) {
        numObj++;
    }
    if (m_showReadMarkers && m_readMarkerItem) {
        implicitHeight += m_readMarkerItem->implicitHeight();
        numObj++;
    }
    implicitHeight += (numObj - 1) * m_spacing;
    implicitHeight += m_showAuthor ? m_spacing * 2 : m_spacing;
    implicitHeight = std::ceil(implicitHeight);
    setImplicitWidth(m_alwaysFillWidth ? m_sizeHelper.maxAvailableWidth() : m_sizeHelper.availableWidth());
    setImplicitHeight(implicitHeight);
}

bool MessageDelegateBase::showMessageOnRight() const
{
    return m_showLocalMessagesOnRight && !m_alwaysFillWidth && m_author && m_author->isLocalMember();
}

void MessageDelegateBase::resizeContent()
{
    if (!isComponentComplete() || m_resizingContent) {
        return;
    }

    m_isDirty = false;
    m_resizingContent = true;

    updateImplicitHeight();

    qreal nextY = m_showAuthor ? m_spacing * 2 : m_spacing;

    if (m_compactMode && m_compactBackgroundItem) {
        m_compactBackgroundItem->setPosition(QPointF(std::ceil(m_sizeHelper.leftX() / 2), std::ceil(nextY / 2)));
        m_compactBackgroundItem->setSize(
            QSizeF(m_sizeHelper.availableWidth() + m_sizeHelper.rightPadding() - std::ceil(m_sizeHelper.leftPadding() / 2), implicitHeight()));
        m_compactBackgroundItem->setZ(-1);
    }
    if (m_showSection && m_sectionItem) {
        m_sectionItem->setPosition(QPointF(m_sizeHelper.leftX(), nextY));
        m_sectionItem->setSize(QSizeF(m_sizeHelper.availableWidth(), m_sectionItem->implicitHeight()));
        nextY += m_sectionItem->implicitHeight() + m_spacing;
    }
    qreal yAdd = 0.0;
    if (showAvatar() && m_avatarItem) {
        m_avatarItem->setPosition(QPointF(m_sizeHelper.leftX(), nextY));
        m_avatarItem->setSize(QSizeF(m_avatarItem->implicitWidth(), m_avatarItem->implicitHeight()));
        yAdd = m_avatarItem->implicitWidth();
    }
    if (m_contentItem) {
        const auto contentItemWidth =
            m_alwaysFillWidth ? m_contentSizeHelper.availableWidth() : std::min(m_contentItem->implicitWidth(), m_contentSizeHelper.availableWidth());
        const auto contentX = showMessageOnRight() ? m_sizeHelper.rightX() - contentItemWidth - 1 : m_contentSizeHelper.leftPadding();
        m_contentItem->setPosition(QPointF(contentX, nextY));
        m_contentItem->setSize(QSizeF(contentItemWidth, m_contentItem->implicitHeight()));
        yAdd = std::max(yAdd, m_contentItem->implicitHeight());
    }
    nextY += yAdd + m_spacing;
    if (m_showReadMarkers && m_readMarkerItem) {
        qreal extraSpacing = m_readMarkerItem->implicitWidth() < m_sizeHelper.availableWidth() - m_spacing ? m_spacing : 0;
        qreal objectWidth = std::min(m_sizeHelper.availableWidth(), m_readMarkerItem->implicitWidth());
        m_readMarkerItem->setPosition(QPointF(m_sizeHelper.rightX() - objectWidth - extraSpacing, nextY));
        m_readMarkerItem->setSize(QSizeF(objectWidth, m_readMarkerItem->implicitHeight()));
    }

    m_resizingContent = false;
}

void MessageDelegateBase::hoverEnterEvent(QHoverEvent *event)
{
    m_hovered = true;
    event->setAccepted(true);
    updateBackground();
}

void MessageDelegateBase::hoverMoveEvent(QHoverEvent *event)
{
    m_hovered = contains(event->pos());
    event->setAccepted(true);
    updateBackground();
}

void MessageDelegateBase::hoverLeaveEvent(QHoverEvent *event)
{
    m_hovered = false;
    event->setAccepted(true);
    updateBackground();
}

bool MessageDelegateBase::isTemporaryHighlighted() const
{
    return m_temporaryHighlightTimer && m_temporaryHighlightTimer->isActive();
}

void MessageDelegateBase::setIsTemporaryHighlighted(bool isTemporaryHighlighted)
{
    if (!isTemporaryHighlighted) {
        if (m_temporaryHighlightTimer) {
            m_temporaryHighlightTimer->stop();
            m_temporaryHighlightTimer->deleteLater();
            Q_EMIT isTemporaryHighlightedChanged();
        }
        return;
    }

    if (!m_temporaryHighlightTimer) {
        m_temporaryHighlightTimer = new QTimer(this);
    }
    m_temporaryHighlightTimer->start(1500);
    connect(m_temporaryHighlightTimer, &QTimer::timeout, this, [this]() {
        m_temporaryHighlightTimer->deleteLater();
        Q_EMIT isTemporaryHighlightedChanged();
    });
    Q_EMIT isTemporaryHighlightedChanged();
}

#include "moc_messagedelegate.cpp"
