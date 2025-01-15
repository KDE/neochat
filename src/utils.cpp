// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "utils.h"

#ifdef HAVE_ICU
#include <QTextBoundaryFinder>
#include <QTextCharFormat>
#include <unicode/uchar.h>
#include <unicode/urename.h>
#endif

#include <Quotient/connection.h>

#include <QJsonDocument>
#include <QQuickWindow>

using namespace Quotient;

bool QmlUtils::isEmoji(const QString &text)
{
    return Utils::isEmoji(text);
}
bool QmlUtils::isValidJson(const QByteArray &json)
{
    return !QJsonDocument::fromJson(json).isNull();
}

QString QmlUtils::escapeString(const QString &string)
{
    return string.toHtmlEscaped();
}

QColor QmlUtils::getUserColor(qreal hueF)
{
    const auto lightness = static_cast<QGuiApplication *>(QGuiApplication::instance())->palette().color(QPalette::Active, QPalette::Window).lightnessF();
    // https://github.com/quotient-im/libQuotient/wiki/User-color-coding-standard-draft-proposal
    return QColor::fromHslF(hueF, 1, -0.7 * lightness + 0.9, 1);
}

QQuickItem *QmlUtils::focusedWindowItem()
{
    const auto window = qobject_cast<QQuickWindow *>(QGuiApplication::focusWindow());
    if (window) {
        return window->contentItem();
    } else {
        return nullptr;
    }
}

bool Utils::isEmoji(const QString &text)
{
#ifdef HAVE_ICU
    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
    int from = 0;
    while (finder.toNextBoundary() != -1) {
        auto to = finder.position();
        if (text[from].isSpace()) {
            from = to;
            continue;
        }

        auto first = text.mid(from, to - from).toUcs4()[0];
        if (!u_hasBinaryProperty(first, UCHAR_EMOJI_PRESENTATION)) {
            return false;
        }
        from = to;
    }
    return true;
#else
    return false;
#endif
}

#include "moc_utils.cpp"
