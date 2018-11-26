#ifndef Utils_H
#define Utils_H

#include "room.h"
#include "user.h"

#include <QObject>
#include <QRegExp>
#include <QString>

#include <events/redactionevent.h>
#include <events/roomavatarevent.h>
#include <events/roommemberevent.h>
#include <events/simplestateevents.h>

namespace utils {
const QRegExp removeReplyRegex{"> <.*>.*\\n\\n"};

QString removeReply(const QString& text);

template <typename BaseEventT>
QString eventToString(const BaseEventT& evt,
                      QMatrixClient::Room* room = nullptr,
                      Qt::TextFormat format = Qt::PlainText) {
  bool prettyPrint = (format == Qt::RichText);

  using namespace QMatrixClient;
  return visit(
      evt,
      [room, prettyPrint](const RoomMessageEvent& e) {
        using namespace MessageEventContent;

        if (prettyPrint && e.hasTextContent() &&
            e.mimeType().name() != "text/plain") {
          static const QRegExp userPillRegExp(
              "<a href=\"https://matrix.to/#/(@.*:.*)\">.*</a>");
          QString formattedStr(
              static_cast<const TextContent*>(e.content())->body);
          int pos = 0;
          while ((pos = userPillRegExp.indexIn(formattedStr, pos)) != -1) {
            QString userId = userPillRegExp.cap(1);
            formattedStr.remove(pos, userPillRegExp.matchedLength());
            formattedStr.insert(pos, "<b class=\"user-pill\">" +
                                         room->user(userId)->displayname() +
                                         "</b>");
            pos += userPillRegExp.matchedLength();
          }
          static const QRegExp codePillRegExp("<pre>(.*)</pre>");
          formattedStr.replace(codePillRegExp, "<i>\\1</i>");
          return formattedStr;
        }
        if (e.hasFileContent()) {
          auto fileCaption = e.content()->fileInfo()->originalName;
          if (fileCaption.isEmpty())
            fileCaption = prettyPrint && room ? room->prettyPrint(e.plainBody())
                                              : e.plainBody();
          if (fileCaption.isEmpty()) return QObject::tr("a file");
        }
        return prettyPrint && room ? room->prettyPrint(e.plainBody())
                                   : e.plainBody();
      },
      [room](const RoomMemberEvent& e) {
        // FIXME: Rewind to the name that was at the time of this event
        QString subjectName =
            room ? room->roomMembername(e.userId()) : e.userId();
        // The below code assumes senderName output in AuthorRole
        switch (e.membership()) {
          case MembershipType::Invite:
            if (e.repeatsState())
              return QObject::tr("reinvited %1 to the room").arg(subjectName);
            FALLTHROUGH;
          case MembershipType::Join: {
            if (e.repeatsState())
              return QObject::tr("joined the room (repeated)");
            if (!e.prevContent() ||
                e.membership() != e.prevContent()->membership) {
              return e.membership() == MembershipType::Invite
                         ? QObject::tr("invited %1 to the room")
                               .arg(subjectName)
                         : QObject::tr("joined the room");
            }
            QString text{};
            if (e.isRename()) {
              if (e.displayName().isEmpty())
                text = QObject::tr("cleared their display name");
              else
                text = QObject::tr("changed their display name to %1")
                           .arg(e.displayName());
            }
            if (e.isAvatarUpdate()) {
              if (!text.isEmpty()) text += " and ";
              if (e.avatarUrl().isEmpty())
                text += QObject::tr("cleared the avatar");
              else
                text += QObject::tr("updated the avatar");
            }
            return text;
          }
          case MembershipType::Leave:
            if (e.prevContent() &&
                e.prevContent()->membership == MembershipType::Ban) {
              return (e.senderId() != e.userId())
                         ? QObject::tr("unbanned %1").arg(subjectName)
                         : QObject::tr("self-unbanned");
            }
            return (e.senderId() != e.userId())
                       ? QObject::tr("has kicked %1 from the room")
                             .arg(subjectName)
                       : QObject::tr("left the room");
          case MembershipType::Ban:
            return (e.senderId() != e.userId())
                       ? QObject::tr("banned %1 from the room ")
                             .arg(subjectName)
                       : QObject::tr(" self-banned from the room ");
          case MembershipType::Knock:
            return QObject::tr("knocked");
          default:;
        }
        return QObject::tr("made something unknown");
      },
      [](const RoomAliasesEvent& e) {
        return QObject::tr("set aliases to: %1").arg(e.aliases().join(","));
      },
      [](const RoomCanonicalAliasEvent& e) {
        return (e.alias().isEmpty())
                   ? QObject::tr("cleared the room main alias")
                   : QObject::tr("set the room main alias to: %1")
                         .arg(e.alias());
      },
      [](const RoomNameEvent& e) {
        return (e.name().isEmpty())
                   ? QObject::tr("cleared the room name")
                   : QObject::tr("set the room name to: %1").arg(e.name());
      },
      [](const RoomTopicEvent& e) {
        return (e.topic().isEmpty())
                   ? QObject::tr("cleared the topic")
                   : QObject::tr("set the topic to: %1").arg(e.topic());
      },
      [](const RoomAvatarEvent&) {
        return QObject::tr("changed the room avatar");
      },
      [](const EncryptionEvent&) {
        return QObject::tr("activated End-to-End Encryption");
      },
      QObject::tr("Unknown Event"));
};
}  // namespace utils

#endif
