#include "utils.h"

QString utils::removeReply(const QString& text) {
  QString result(text);
  result.remove(utils::removeRichReplyRegex);
  result.remove(utils::removeReplyRegex);
  return result;
}
