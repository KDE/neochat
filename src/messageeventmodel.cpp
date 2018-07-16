#include "messageeventmodel.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtQml>  // for qmlRegisterType()

#include "events/redactionevent.h"
#include "events/roomavatarevent.h"
#include "events/roommemberevent.h"
#include "events/simplestateevents.h"

#include "connection.h"
#include "settings.h"
#include "user.h"

MessageEventModel::MessageEventModel(QObject* parent)
    : QAbstractListModel(parent) {
  qmlRegisterType<QMatrixClient::FileTransferInfo>();
  qRegisterMetaType<QMatrixClient::FileTransferInfo>();
}

MessageEventModel::~MessageEventModel() {}

void MessageEventModel::setRoom(QMatrixClient::Room* room) {
  if (room == m_currentRoom) return;

  beginResetModel();
  if (m_currentRoom) {
    m_currentRoom->disconnect(this);
    qDebug() << "Disconnected from" << m_currentRoom->id();
  }

  m_currentRoom = room;
  if (room) {
    lastReadEventId = room->readMarkerEventId();
    using namespace QMatrixClient;
    connect(m_currentRoom, &Room::aboutToAddNewMessages, this,
            [=](RoomEventsRange events) {
              beginInsertRows(QModelIndex(), 0, int(events.size()) - 1);
            });
    connect(m_currentRoom, &Room::aboutToAddHistoricalMessages, this,
            [=](RoomEventsRange events) {
              if (rowCount() > 0) nextNewerRow = rowCount() - 1;
              beginInsertRows(QModelIndex(), rowCount(),
                              rowCount() + int(events.size()) - 1);
            });
    connect(m_currentRoom, &Room::addedMessages, this, [=] {
      if (nextNewerRow > -1) {
        const auto idx = index(nextNewerRow);
        emit dataChanged(idx, idx);
        nextNewerRow = -1;
      }
      endInsertRows();
    });
    connect(m_currentRoom, &Room::readMarkerMoved, this, [this] {
      refreshEventRoles(
          std::exchange(lastReadEventId, m_currentRoom->readMarkerEventId()),
          {ReadMarkerRole});
      refreshEventRoles(lastReadEventId, {ReadMarkerRole});
    });
    connect(
        m_currentRoom, &Room::replacedEvent, this,
        [this](const RoomEvent* newEvent) { refreshEvent(newEvent->id()); });
    connect(m_currentRoom, &Room::fileTransferProgress, this,
            &MessageEventModel::refreshEvent);
    connect(m_currentRoom, &Room::fileTransferCompleted, this,
            &MessageEventModel::refreshEvent);
    connect(m_currentRoom, &Room::fileTransferFailed, this,
            &MessageEventModel::refreshEvent);
    connect(m_currentRoom, &Room::fileTransferCancelled, this,
            &MessageEventModel::refreshEvent);
    qDebug() << "Connected to room" << room->id() << "as"
             << room->localUser()->id();
  } else
    lastReadEventId.clear();
  endResetModel();
  emit roomChanged();
}

void MessageEventModel::refreshEvent(const QString& eventId) {
  refreshEventRoles(eventId, {});
}

void MessageEventModel::refreshEventRoles(const QString& eventId,
                                          const QVector<int> roles) {
  const auto it = m_currentRoom->findInTimeline(eventId);
  if (it != m_currentRoom->timelineEdge()) {
    const auto row = it - m_currentRoom->messageEvents().rbegin();
    emit dataChanged(index(row), index(row), roles);
  }
}

inline bool hasValidTimestamp(const QMatrixClient::TimelineItem& ti) {
  return ti->timestamp().isValid();
}

QDateTime MessageEventModel::makeMessageTimestamp(
    QMatrixClient::Room::rev_iter_t baseIt) const {
  const auto& timeline = m_currentRoom->messageEvents();
  auto ts = baseIt->event()->timestamp();
  if (ts.isValid()) return ts;

  // The event is most likely redacted or just invalid.
  // Look for the nearest date around and slap zero time to it.
  using QMatrixClient::TimelineItem;
  auto rit = std::find_if(baseIt, timeline.rend(), hasValidTimestamp);
  if (rit != timeline.rend())
    return {rit->event()->timestamp().date(), {0, 0}, Qt::LocalTime};
  auto it = std::find_if(baseIt.base(), timeline.end(), hasValidTimestamp);
  if (it != timeline.end())
    return {it->event()->timestamp().date(), {0, 0}, Qt::LocalTime};

  // What kind of room is that?..
  qCritical() << "No valid timestamps in the room timeline!";
  return {};
}

QString MessageEventModel::makeDateString(
    QMatrixClient::Room::rev_iter_t baseIt) const {
  auto date = makeMessageTimestamp(baseIt).toLocalTime().date();
  if (QMatrixClient::SettingsGroup("UI")
          .value("banner_human_friendly_date", true)
          .toBool()) {
    if (date == QDate::currentDate()) return tr("Today");
    if (date == QDate::currentDate().addDays(-1)) return tr("Yesterday");
    if (date == QDate::currentDate().addDays(-2))
      return tr("The day before yesterday");
    if (date > QDate::currentDate().addDays(-7)) return date.toString("dddd");
  }
  return date.toString(Qt::DefaultLocaleShortDate);
}

int MessageEventModel::rowCount(const QModelIndex& parent) const {
  if (!m_currentRoom || parent.isValid()) return 0;
  return m_currentRoom->timelineSize();
}

QVariant MessageEventModel::data(const QModelIndex& index, int role) const {
  if (!m_currentRoom || index.row() < 0 ||
      index.row() >= m_currentRoom->timelineSize())
    return QVariant();

  const auto timelineIt = m_currentRoom->messageEvents().rbegin() + index.row();
  const auto& ti = *timelineIt;

  using namespace QMatrixClient;
  if (role == Qt::DisplayRole) {
    if (ti->isRedacted()) {
      auto reason = ti->redactedBecause()->reason();
      if (reason.isEmpty())
        return tr("Redacted");
      else
        return tr("Redacted: %1").arg(ti->redactedBecause()->reason());
    }

    if (ti->type() == EventType::RoomMessage) {
      using namespace MessageEventContent;

      auto* e = ti.viewAs<RoomMessageEvent>();
      if (e->hasTextContent() && e->mimeType().name() != "text/plain")
        return static_cast<const TextContent*>(e->content())->body;
      if (e->hasFileContent()) {
        auto fileCaption = e->content()->fileInfo()->originalName;
        if (fileCaption.isEmpty())
          fileCaption = m_currentRoom->prettyPrint(e->plainBody());
        if (fileCaption.isEmpty()) return tr("a file");
      }
      return m_currentRoom->prettyPrint(e->plainBody());
    }
    if (ti->type() == EventType::RoomMember) {
      auto* e = ti.viewAs<RoomMemberEvent>();
      // FIXME: Rewind to the name that was at the time of this event
      QString subjectName = m_currentRoom->roomMembername(e->userId());
      // The below code assumes senderName output in AuthorRole
      switch (e->membership()) {
        case MembershipType::Invite:
          if (e->repeatsState())
            return tr("reinvited %1 to the room").arg(subjectName);
          // [[fallthrough]]
        case MembershipType::Join: {
          if (e->repeatsState()) return tr("joined the room (repeated)");
          if (!e->prev_content() ||
              e->membership() != e->prev_content()->membership) {
            return e->membership() == MembershipType::Invite
                       ? tr("invited %1 to the room").arg(subjectName)
                       : tr("joined the room");
          }
          QString text{};
          if (e->displayName() != e->prev_content()->displayName) {
            if (e->displayName().isEmpty())
              text = tr("cleared the display name");
            else
              text = tr("changed the display name to %1").arg(e->displayName());
          }
          if (e->avatarUrl() != e->prev_content()->avatarUrl) {
            if (!text.isEmpty()) text += " and ";
            if (e->avatarUrl().isEmpty())
              text += tr("cleared the avatar");
            else
              text += tr("updated the avatar");
          }
          return text;
        }
        case MembershipType::Leave:
          if (e->prev_content() &&
              e->prev_content()->membership == MembershipType::Ban) {
            if (e->senderId() != e->userId())
              return tr("unbanned %1").arg(subjectName);
            else
              return tr("self-unbanned");
          }
          if (e->senderId() != e->userId())
            return tr("has put %1 out of the room").arg(subjectName);
          else
            return tr("left the room");
        case MembershipType::Ban:
          if (e->senderId() != e->userId())
            return tr("banned %1 from the room").arg(subjectName);
          else
            return tr("self-banned from the room");
        case MembershipType::Knock:
          return tr("knocked");
        case MembershipType::Undefined:
          return tr("made something unknown");
      }
    }
    if (ti->type() == EventType::RoomAliases) {
      auto* e = ti.viewAs<RoomAliasesEvent>();
      return tr("set aliases to: %1").arg(e->aliases().join(", "));
    }
    if (ti->type() == EventType::RoomCanonicalAlias) {
      auto* e = ti.viewAs<RoomCanonicalAliasEvent>();
      if (e->alias().isEmpty())
        return tr("cleared the room main alias");
      else
        return tr("set the room main alias to: %1").arg(e->alias());
    }
    if (ti->type() == EventType::RoomName) {
      auto* e = ti.viewAs<RoomNameEvent>();
      if (e->name().isEmpty())
        return tr("cleared the room name");
      else
        return tr("set the room name to: %1").arg(e->name());
    }
    if (ti->type() == EventType::RoomTopic) {
      auto* e = ti.viewAs<RoomTopicEvent>();
      if (e->topic().isEmpty())
        return tr("cleared the topic");
      else
        return tr("set the topic to: %1").arg(e->topic());
    }
    if (ti->type() == EventType::RoomAvatar) {
      return tr("changed the room avatar");
    }
    if (ti->type() == EventType::RoomEncryption) {
      return tr("activated End-to-End Encryption");
    }
    return tr("Unknown Event");
  }

  if (role == Qt::ToolTipRole) {
    return ti->originalJson();
  }

  if (role == EventTypeRole) {
    if (ti->isStateEvent()) return "state";

    if (ti->type() == EventType::RoomMessage) {
      switch (ti.viewAs<RoomMessageEvent>()->msgtype()) {
        case MessageEventType::Emote:
          return "emote";
        case MessageEventType::Notice:
          return "notice";
        case MessageEventType::Image:
          return "image";
        case MessageEventType::Audio:
//          return "audio";
        case MessageEventType::File:
        case MessageEventType::Video:
          return "file";
        default:
          return "message";
      }
    }

    return "other";
  }

  if (role == TimeRole) return makeMessageTimestamp(timelineIt);

  if (role == SectionRole)
    return makeDateString(timelineIt);  // FIXME: move date rendering to QML

  if (role == AuthorRole) {
    auto userId = ti->senderId();
    // FIXME: It shouldn't be User, it should be its state "as of event"
    return QVariant::fromValue(m_currentRoom->user(userId));
  }

  if (role == ContentTypeRole) {
    if (ti->type() == EventType::RoomMessage) {
      const auto& contentType =
          ti.viewAs<RoomMessageEvent>()->mimeType().name();
      return contentType == "text/plain" ? "text/html" : contentType;
    }
    return "text/plain";
  }

  if (role == ContentRole) {
    if (ti->isRedacted()) {
      auto reason = ti->redactedBecause()->reason();
      if (reason.isEmpty())
        return tr("Redacted");
      else
        return tr("Redacted: %1").arg(ti->redactedBecause()->reason());
    }

    if (ti->type() == EventType::RoomMessage) {
      using namespace MessageEventContent;

      auto* e = ti.viewAs<RoomMessageEvent>();
      switch (e->msgtype()) {
        case MessageEventType::Image:
        case MessageEventType::File:
        case MessageEventType::Audio:
        case MessageEventType::Video:
          return QVariant::fromValue(e->content()->originalJson);
        default:;
      }
    }
  }

  if (role == ReadMarkerRole) return ti->id() == lastReadEventId;

  if (role == SpecialMarksRole) {
    if (ti->isStateEvent() && ti.viewAs<StateEventBase>()->repeatsState())
      return "hidden";
    return ti->isRedacted() ? "redacted" : "";
  }

  if (role == EventIdRole) return ti->id();

  if (role == LongOperationRole) {
    if (ti->type() == EventType::RoomMessage &&
        ti.viewAs<RoomMessageEvent>()->hasFileContent()) {
      auto info = m_currentRoom->fileTransferInfo(ti->id());
      return QVariant::fromValue(info);
    }
  }

  auto aboveEventIt = timelineIt + 1;  // FIXME: shouldn't be here, because #312
  if (aboveEventIt != m_currentRoom->timelineEdge()) {
    if (role == AboveSectionRole) return makeDateString(aboveEventIt);

    if (role == AboveAuthorRole)
      return QVariant::fromValue(
          m_currentRoom->user((*aboveEventIt)->senderId()));
  }

  return QVariant();
}

QHash<int, QByteArray> MessageEventModel::roleNames() const {
  QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
  roles[EventTypeRole] = "eventType";
  roles[EventIdRole] = "eventId";
  roles[TimeRole] = "time";
  roles[SectionRole] = "section";
  roles[AboveSectionRole] = "aboveSection";
  roles[AuthorRole] = "author";
  roles[AboveAuthorRole] = "aboveAuthor";
  roles[ContentRole] = "content";
  roles[ContentTypeRole] = "contentType";
  roles[HighlightRole] = "highlight";
  roles[ReadMarkerRole] = "readMarker";
  roles[SpecialMarksRole] = "marks";
  roles[LongOperationRole] = "progressInfo";
  roles[EventResolvedTypeRole] = "eventResolvedType";
  return roles;
}
