/**
 * SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
#ifndef Utils_H
#define Utils_H

#include "room.h"
#include "user.h"

#include <QObject>
#include <QRegularExpression>
#include <QString>

#include <events/redactionevent.h>
#include <events/roomavatarevent.h>
#include <events/roommemberevent.h>
#include <events/simplestateevents.h>

namespace utils
{
static const QRegularExpression removeReplyRegex{"> <.*?>.*?\\n\\n", QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression removeRichReplyRegex{"<mx-reply>.*?</mx-reply>", QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression codePillRegExp{"<pre><code[^>]*>(.*?)</code></pre>", QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression userPillRegExp{"<a href=\"https://matrix.to/#/@.*?:.*?\">(.*?)</a>", QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression strikethroughRegExp{"<del>(.*?)</del>", QRegularExpression::DotMatchesEverythingOption};
} // namespace utils

#endif
