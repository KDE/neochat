#include "utils.h"

QString utils::removeReply(const QString& text) {
  QString result(text);
  return result.remove(utils::removeReplyRegex);
}
