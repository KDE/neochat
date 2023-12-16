// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <QColor>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlEngine>
#include <QRegularExpression>

#include <Quotient/connection.h>
#include <Quotient/user.h>

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

    Q_INVOKABLE QVariantMap getUser(Quotient::User *user) const;

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
}

namespace TextRegex
{
static const QRegularExpression endTagType{QStringLiteral("(>| )")};
static const QRegularExpression attributeData{QStringLiteral("['\"](.*?)['\"]")};
static const QRegularExpression removeReply{QStringLiteral("> <.*?>.*?\\n\\n"), QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression removeRichReply{QStringLiteral("<mx-reply>.*?</mx-reply>"), QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression codePill{QStringLiteral("<pre><code[^>]*>(.*?)</code></pre>"), QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression userPill{QStringLiteral("(<a href=\"https://matrix.to/#/@.*?:.*?\">.*?</a>)"), QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression blockQuote{QStringLiteral("<blockquote>\n?(?:<p>)?(.*?)(?:</p>)?\n?</blockquote>"),
                                           QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression strikethrough{QStringLiteral("<del>(.*?)</del>"), QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression mxcImage{QStringLiteral(R"AAA(<img(.*?)src="mxc:\/\/(.*?)\/(.*?)"(.*?)>)AAA")};
static const QRegularExpression plainUrl(
    QStringLiteral(
        R"(<a.*?<\/a>(*SKIP)(*F)|\b((www\.(?!\.)(?!(\w|\.|-)+@)|(https?|ftp):(//)?\w|(magnet|matrix):)(&(?![lg]t;)|[^&\s<>'"])+(&(?![lg]t;)|[^?&!,.\s<>'"\]):])))"),
    QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
static const QRegularExpression
    url(QStringLiteral(R"(\b((www\.(?!\.)(?!(\w|\.|-)+@)|https?:(//)?\w)(&(?![lg]t;)|[^&\s<>'"])+(&(?![lg]t;)|[^&!,.\s<>'"\]):])))"),
        QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
static const QRegularExpression emailAddress(QStringLiteral(R"(<a.*?<\/a>(*SKIP)(*F)|\b(mailto:)?((\w|\.|-)+@(\w|\.|-)+\.\w+\b))"),
                                             QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
static const QRegularExpression mxId(QStringLiteral(R"((^|[][[:space:](){}`'";])([!#@][-a-z0-9_=#/.]{1,252}:\w(?:\w|\.|-)*\.\w+(?::\d{1,5})?))"),
                                     QRegularExpression::CaseInsensitiveOption | QRegularExpression::UseUnicodePropertiesOption);
}
