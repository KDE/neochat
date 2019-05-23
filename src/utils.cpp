#include "utils.h"

QString utils::removeReply(const QString& text) {
  QString result(text);
  result.remove(utils::removeRichReplyRegex);
  result.remove(utils::removeReplyRegex);
  return result;
}

QString utils::cleanHTML(const QString& text) {
  QString result(text);
  result.replace(codePillRegExp, "<i>\\1</i>");
  result.replace(userPillRegExp, "<b>\\1</b>");
  return result;
}
