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

namespace utils {
static const QRegularExpression removeReplyRegex{
    "> <.*?>.*?\\n\\n", QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression removeRichReplyRegex{
    "<mx-reply>.*?</mx-reply>", QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression codePillRegExp{
    "<pre>(.*?)</pre>", QRegularExpression::DotMatchesEverythingOption};
static const QRegularExpression userPillRegExp{
    "<a href=\"https://matrix.to/#/@.*?:.*?\">(.*?)</a>",
    QRegularExpression::DotMatchesEverythingOption};

QString removeReply(const QString& text);
QString cleanHTML(const QString& text, QMatrixClient::Room* room);

}  // namespace utils

#endif
