// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QColor>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlEngine>
#include <QQuickItem>
#include <QRegularExpression>

#include <Quotient/user.h>

#include "enums/powerlevel.h"

using namespace Qt::StringLiterals;

class QmlUtils : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static QmlUtils *create(QQmlEngine *, QJSEngine *)
    {
        QQmlEngine::setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

    static QmlUtils &instance()
    {
        static QmlUtils _instance;
        return _instance;
    }

    Q_INVOKABLE bool isEmoji(const QString &text);
    Q_INVOKABLE bool isValidJson(const QByteArray &json);
    Q_INVOKABLE QString escapeString(const QString &string);
    Q_INVOKABLE QColor getUserColor(qreal hueF);
    Q_INVOKABLE QQuickItem *focusedWindowItem();
    /**
     * @brief Invokable version of PowerLevel::nameForLevel which also calls PowerLevel::levelForValue.
     */
    Q_INVOKABLE QString nameForPowerLevelValue(int value);

private:
    QmlUtils() = default;
};

namespace Utils
{

/**
 * @brief Get a color for a user from a hueF value.
 *
 * The lightness of the color will be modified depending on the current palette in
 * order to maintain contrast.
 */
inline QColor getUserColor(qreal hueF)
{
    const auto lightness = static_cast<QGuiApplication *>(QGuiApplication::instance())->palette().color(QPalette::Active, QPalette::Window).lightnessF();
    // https://github.com/quotient-im/libQuotient/wiki/User-color-coding-standard-draft-proposal
    return QColor::fromHslF(hueF, 1, -0.7 * lightness + 0.9, 1);
}

bool isEmoji(const QString &text);
}

namespace TextRegex
{
static const QRegularExpression endTagType{u"[> /]"_s};
static const QRegularExpression endAttributeType{u"[> ]"_s};
static const QRegularExpression attributeData{u"['\"](.*?)['\"]"_s};
static const QRegularExpression removeReply{u"> <.*?>.*?\\n\\n"_s, QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression removeRichReply{u"<mx-reply>.*?</mx-reply>"_s, QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression codePill{u"<pre><code[^>]*>(.*?)</code></pre>"_s, QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression userPill{u"(<a href=\"https://matrix.to/#/@.*?:.*?\">.*?</a>)"_s, QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression blockQuote{u"<blockquote>\n?(?:<p>)?(.*?)(?:</p>)?\n?</blockquote>"_s, QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression strikethrough{u"<del>(.*?)</del>"_s, QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression mxcImage{uR"AAA(<img(.*?)src="mxc:\/\/(.*?)\/(.*?)"(.*?)>)AAA"_s};
static const QRegularExpression plainUrl(
    uR"(<a.*?<\/a>(*SKIP)(*F)|\b((www\.(?!\.)(?!(\w|\.|-)+@)|(https?|ftp):(//)?\w|(magnet|matrix):)(&(?![lg]t;)|[^&\s<>'"])+(&(?![lg]t;)|[^?&!,.\s<>'"\]):])))"_s,
    QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
static const QRegularExpression url(uR"(\b((www\.(?!\.)(?!(\w|\.|-)+@)|https?:(//)?\w)(&(?![lg]t;)|[^&\s<>'"])+(&(?![lg]t;)|[^&!,.\s<>'"\]):])))"_s,
                                    QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
static const QRegularExpression emailAddress(uR"(<a.*?<\/a>(*SKIP)(*F)|\b(mailto:)?((\w|\.|-)+@(\w|\.|-)+\.\w+\b))"_s,
                                             QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
static const QRegularExpression mxId(uR"((?<=^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\w+(?::\d{1,5})?))"_s,
                                     QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
}
