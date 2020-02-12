#include "messageeventmodel.h"

#include <connection.h>
#include <events/reactionevent.h>
#include <events/redactionevent.h>
#include <events/roomavatarevent.h>
#include <events/roommemberevent.h>
#include <events/simplestateevents.h>
#include <settings.h>
#include <user.h>

#include <QtCore/QDebug>
#include <QtQml>  // for qmlRegisterType()

#include "utils.h"

QHash<int, QByteArray> MessageEventModel::roleNames() const {
  QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
  roles[EventTypeRole] = "eventType";
  roles[MessageRole] = "message";
  roles[EventIdRole] = "eventId";
  roles[TimeRole] = "time";
  roles[SectionRole] = "section";
  roles[AuthorRole] = "author";
  roles[ContentRole] = "content";
  roles[ContentTypeRole] = "contentType";
  roles[HighlightRole] = "highlight";
  roles[ReadMarkerRole] = "readMarker";
  roles[SpecialMarksRole] = "marks";
  roles[LongOperationRole] = "progressInfo";
  roles[AnnotationRole] = "annotation";
  roles[EventResolvedTypeRole] = "eventResolvedType";
  roles[ReplyRole] = "reply";
  roles[UserMarkerRole] = "userMarker";
  roles[ShowAuthorRole] = "showAuthor";
  roles[ShowSectionRole] = "showSection";
  roles[ReactionRole] = "reaction";
  return roles;
}

MessageEventModel::MessageEventModel(QObject* parent)
    : QAbstractListModel(parent), m_currentRoom(nullptr) {
  using namespace Quotient;
  qmlRegisterType<FileTransferInfo>();
  qRegisterMetaType<FileTransferInfo>();
  qmlRegisterUncreatableType<EventStatus>(
      "Spectral", 0, 1, "EventStatus", "EventStatus is not an creatable type");
}

MessageEventModel::~MessageEventModel() {}

void MessageEventModel::setRoom(SpectralRoom* room) {
  if (room == m_currentRoom)
    return;

  beginResetModel();
  if (m_currentRoom) {
    m_currentRoom->disconnect(this);
  }

  m_currentRoom = room;
  if (room) {
    lastReadEventId = room->readMarkerEventId();

    using namespace Quotient;
    connect(m_currentRoom, &Room::aboutToAddNewMessages, this,
            [=](RoomEventsRange events) {
              beginInsertRows({}, timelineBaseIndex(),
                              timelineBaseIndex() + int(events.size()) - 1);
            });
    connect(m_currentRoom, &Room::aboutToAddHistoricalMessages, this,
            [=](RoomEventsRange events) {
              if (rowCount() > 0)
                rowBelowInserted = rowCount() - 1;  // See #312
              beginInsertRows({}, rowCount(),
                              rowCount() + int(events.size()) - 1);
            });
    connect(m_currentRoom, &Room::addedMessages, this,
            [=](int lowest, int biggest) {
              endInsertRows();
              if (biggest < m_currentRoom->maxTimelineIndex()) {
                auto rowBelowInserted = m_currentRoom->maxTimelineIndex() -
                                        biggest + timelineBaseIndex() - 1;
                refreshEventRoles(rowBelowInserted,
                                  {ShowAuthorRole});
              }
              for (auto i = m_currentRoom->maxTimelineIndex() - biggest;
                   i <= m_currentRoom->maxTimelineIndex() - lowest; ++i)
                refreshLastUserEvents(i);
            });
    connect(m_currentRoom, &Room::pendingEventAboutToAdd, this,
            [this] { beginInsertRows({}, 0, 0); });
    connect(m_currentRoom, &Room::pendingEventAdded, this, &MessageEventModel::endInsertRows);
    connect(m_currentRoom, &Room::pendingEventAboutToMerge, this,
            [this](RoomEvent*, int i) {
              if (i == 0)
                return;  // No need to move anything, just refresh

              movingEvent = true;
              // Reverse i because row 0 is bottommost in the model
              const auto row = timelineBaseIndex() - i - 1;
              Q_ASSERT(beginMoveRows({}, row, row, {}, timelineBaseIndex()));
            });
    connect(m_currentRoom, &Room::pendingEventMerged, this, [this] {
      if (movingEvent) {
        endMoveRows();
        movingEvent = false;
      }
      refreshRow(timelineBaseIndex());  // Refresh the looks
      refreshLastUserEvents(0);
      if (m_currentRoom->timelineSize() > 1)  // Refresh above
        refreshEventRoles(timelineBaseIndex() + 1, {ReadMarkerRole});
      if (timelineBaseIndex() > 0)  // Refresh below, see #312
        refreshEventRoles(timelineBaseIndex() - 1,
                          {ShowAuthorRole});
    });
    connect(m_currentRoom, &Room::pendingEventChanged, this,
            &MessageEventModel::refreshRow);
    connect(m_currentRoom, &Room::pendingEventAboutToDiscard, this,
            [this](int i) { beginRemoveRows({}, i, i); });
    connect(m_currentRoom, &Room::pendingEventDiscarded, this,
            &MessageEventModel::endRemoveRows);
    connect(m_currentRoom, &Room::readMarkerMoved, this, [this] {
      refreshEventRoles(
          std::exchange(lastReadEventId, m_currentRoom->readMarkerEventId()),
          {ReadMarkerRole});
      refreshEventRoles(lastReadEventId, {ReadMarkerRole});
    });
    connect(m_currentRoom, &Room::replacedEvent, this,
            [this](const RoomEvent* newEvent) {
              refreshLastUserEvents(refreshEvent(newEvent->id()) -
                                    timelineBaseIndex());
            });
    connect(m_currentRoom, &Room::updatedEvent, this,
            [this](const QString& eventId) {
              if (eventId.isEmpty()) {  // How did we get here?
                return;
              }
              refreshEventRoles(eventId, {ReactionRole, Qt::DisplayRole});
            });
    connect(m_currentRoom, &Room::fileTransferProgress, this,
            &MessageEventModel::refreshEvent);
    connect(m_currentRoom, &Room::fileTransferCompleted, this,
            &MessageEventModel::refreshEvent);
    connect(m_currentRoom, &Room::fileTransferFailed, this,
            &MessageEventModel::refreshEvent);
    connect(m_currentRoom, &Room::fileTransferCancelled, this,
            &MessageEventModel::refreshEvent);
    connect(m_currentRoom, &Room::readMarkerForUserMoved, this,
            [=](User*, QString fromEventId, QString toEventId) {
              refreshEventRoles(fromEventId, {UserMarkerRole});
              refreshEventRoles(toEventId, {UserMarkerRole});
            });
    connect(m_currentRoom->connection(), &Connection::ignoredUsersListChanged,
            this, [=] {
              beginResetModel();
              endResetModel();
            });
    qDebug() << "Connected to room" << room->id() << "as"
             << room->localUser()->id();
  } else
    lastReadEventId.clear();
  endResetModel();
}

int MessageEventModel::refreshEvent(const QString& eventId) {
  return refreshEventRoles(eventId);
}

void MessageEventModel::refreshRow(int row) {
  refreshEventRoles(row);
}

int MessageEventModel::timelineBaseIndex() const {
  return m_currentRoom ? int(m_currentRoom->pendingEvents().size()) : 0;
}

void MessageEventModel::refreshEventRoles(int row, const QVector<int>& roles) {
  const auto idx = index(row);
  emit dataChanged(idx, idx, roles);
}

int MessageEventModel::refreshEventRoles(const QString& id,
                                         const QVector<int>& roles) {
  // On 64-bit platforms, difference_type for std containers is long long
  // but Qt uses int throughout its interfaces; hence casting to int below.
  int row = -1;
  // First try pendingEvents because it is almost always very short.
  const auto pendingIt = m_currentRoom->findPendingEvent(id);
  if (pendingIt != m_currentRoom->pendingEvents().end())
    row = int(pendingIt - m_currentRoom->pendingEvents().begin());
  else {
    const auto timelineIt = m_currentRoom->findInTimeline(id);
    if (timelineIt == m_currentRoom->timelineEdge()) {
      qWarning() << "Trying to refresh inexistent event:" << id;
      return -1;
    }
    row = int(timelineIt - m_currentRoom->messageEvents().rbegin()) +
          timelineBaseIndex();
  }
  refreshEventRoles(row, roles);
  return row;
}

inline bool hasValidTimestamp(const Quotient::TimelineItem& ti) {
  return ti->timestamp().isValid();
}

QDateTime MessageEventModel::makeMessageTimestamp(
    const Quotient::Room::rev_iter_t& baseIt) const {
  const auto& timeline = m_currentRoom->messageEvents();
  auto ts = baseIt->event()->timestamp();
  if (ts.isValid())
    return ts;

  // The event is most likely redacted or just invalid.
  // Look for the nearest date around and slap zero time to it.
  using Quotient::TimelineItem;
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

QString MessageEventModel::renderDate(QDateTime timestamp) const {
  auto date = timestamp.toLocalTime().date();
  if (date == QDate::currentDate())
    return tr("Today");
  if (date == QDate::currentDate().addDays(-1))
    return tr("Yesterday");
  if (date == QDate::currentDate().addDays(-2))
    return tr("The day before yesterday");
  if (date > QDate::currentDate().addDays(-7))
    return date.toString("dddd");
  return date.toString(Qt::DefaultLocaleShortDate);
}

void MessageEventModel::refreshLastUserEvents(int baseTimelineRow) {
  if (!m_currentRoom || m_currentRoom->timelineSize() <= baseTimelineRow)
    return;

  const auto& timelineBottom = m_currentRoom->messageEvents().rbegin();
  const auto& lastSender = (*(timelineBottom + baseTimelineRow))->senderId();
  const auto limit = timelineBottom + std::min(baseTimelineRow + 10,
                                               m_currentRoom->timelineSize());
  for (auto it = timelineBottom + std::max(baseTimelineRow - 10, 0);
       it != limit; ++it) {
    if ((*it)->senderId() == lastSender) {
      auto idx = index(it - timelineBottom);
      emit dataChanged(idx, idx);
    }
  }
}

int MessageEventModel::rowCount(const QModelIndex& parent) const {
  if (!m_currentRoom || parent.isValid())
    return 0;
  return m_currentRoom->timelineSize();
}

inline QVariantMap userAtEvent(SpectralUser* user,
                               SpectralRoom* room,
                               const RoomEvent& evt) {
  return QVariantMap{
      {"isLocalUser", user->id() == room->localUser()->id()},
      {"id", user->id()},
      {"avatarMediaId", user->avatarMediaId(room)},
      {"avatarUrl", user->avatarUrl(room)},
      {"displayName", user->displayname(room)},
      {"color", user->color()},
      {"object", QVariant::fromValue(user)},
  };
}

QVariant MessageEventModel::data(const QModelIndex& idx, int role) const {
  const auto row = idx.row();

  if (!m_currentRoom || row < 0 ||
      row >= int(m_currentRoom->pendingEvents().size()) +
                 m_currentRoom->timelineSize())
    return {};

  bool isPending = row < timelineBaseIndex();
  const auto timelineIt = m_currentRoom->messageEvents().crbegin() +
                          std::max(0, row - timelineBaseIndex());
  const auto pendingIt = m_currentRoom->pendingEvents().crbegin() +
                         std::min(row, timelineBaseIndex());
  const auto& evt = isPending ? **pendingIt : **timelineIt;

  if (role == Qt::DisplayRole) {
    return m_currentRoom->eventToString(evt, Qt::RichText);
  }

  if (role == MessageRole) {
    return m_currentRoom->eventToString(evt);
  }

  if (role == Qt::ToolTipRole) {
    return evt.originalJson();
  }

  if (role == EventTypeRole) {
    if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
      switch (e->msgtype()) {
        case MessageEventType::Emote:
          return "emote";
        case MessageEventType::Notice:
          return "notice";
        case MessageEventType::Image:
          return "image";
        case MessageEventType::Audio:
          return "audio";
        case MessageEventType::Video:
          return "video";
        default:
          break;
      }
      if (e->hasFileContent()) {
        return "file";
      }

      return "message";
    }
    if (evt.isStateEvent())
      return "state";

    return "other";
  }

  if (role == EventResolvedTypeRole)
    return EventTypeRegistry::getMatrixType(evt.type());

  if (role == AuthorRole) {
    auto author = static_cast<SpectralUser*>(
        isPending ? m_currentRoom->localUser()
                  : m_currentRoom->user(evt.senderId()));
    return userAtEvent(author, m_currentRoom, evt);
  }

  if (role == ContentTypeRole) {
    if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
      const auto& contentType = e->mimeType().name();
      return contentType == "text/plain" ? QStringLiteral("text/html")
                                         : contentType;
    }
    return QStringLiteral("text/plain");
  }

  if (role == ContentRole) {
    if (evt.isRedacted()) {
      auto reason = evt.redactedBecause()->reason();
      return (reason.isEmpty())
                 ? tr("[REDACTED]")
                 : tr("[REDACTED: %1]").arg(evt.redactedBecause()->reason());
    }

    if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
      // Cannot use e.contentJson() here because some
      // EventContent classes inject values into the copy of the
      // content JSON stored in EventContent::Base
      return e->hasFileContent()
                 ? QVariant::fromValue(e->content()->originalJson)
                 : QVariant();
    };
  }

  if (role == HighlightRole)
    return m_currentRoom->isEventHighlighted(&evt);

  if (role == ReadMarkerRole)
    return evt.id() == lastReadEventId && row > timelineBaseIndex();

  if (role == SpecialMarksRole) {
    if (isPending)
      return pendingIt->deliveryStatus();

    auto* memberEvent = timelineIt->viewAs<RoomMemberEvent>();
    if (memberEvent) {
      if ((memberEvent->isJoin() || memberEvent->isLeave()) &&
          !Settings().value("UI/show_joinleave", true).toBool())
        return EventStatus::Hidden;
    }

    if (is<RedactionEvent>(evt) || is<ReactionEvent>(evt))
      return EventStatus::Hidden;
    if (evt.isRedacted())
      return EventStatus::Hidden;

    if (evt.isStateEvent() &&
        static_cast<const StateEventBase&>(evt).repeatsState())
      return EventStatus::Hidden;

    if (auto e = eventCast<const RoomMessageEvent>(&evt)) {
      if (!e->replacedEvent().isEmpty() && e->replacedEvent() != e->id()) {
        return EventStatus::Hidden;
      }
    }

    if (m_currentRoom->connection()->isIgnored(
            m_currentRoom->user(evt.senderId())))
      return EventStatus::Hidden;

    return EventStatus::Normal;
  }

  if (role == EventIdRole)
    return !evt.id().isEmpty() ? evt.id() : evt.transactionId();

  if (role == LongOperationRole) {
    if (auto e = eventCast<const RoomMessageEvent>(&evt))
      if (e->hasFileContent())
        return QVariant::fromValue(m_currentRoom->fileTransferInfo(e->id()));
  }

  if (role == AnnotationRole)
    if (isPending)
      return pendingIt->annotation();

  if (role == TimeRole || role == SectionRole) {
    auto ts =
        isPending ? pendingIt->lastUpdated() : makeMessageTimestamp(timelineIt);
    return role == TimeRole ? QVariant(ts) : renderDate(ts);
  }

  if (role == UserMarkerRole) {
    QVariantList variantList;
    for (User* user : m_currentRoom->usersAtEventId(evt.id())) {
      if (user == m_currentRoom->localUser())
        continue;
      variantList.append(QVariant::fromValue(user));
    }
    return variantList;
  }

  if (role == ReplyRole) {
    const QString& replyEventId = evt.contentJson()["m.relates_to"]
                                      .toObject()["m.in_reply_to"]
                                      .toObject()["event_id"]
                                      .toString();
    if (replyEventId.isEmpty())
      return {};
    const auto replyIt = m_currentRoom->findInTimeline(replyEventId);
    if (replyIt == m_currentRoom->timelineEdge())
      return {};
    const auto& replyEvt = **replyIt;

    return QVariantMap{
        {"eventId", replyEventId},
        {"display", m_currentRoom->eventToString(replyEvt, Qt::RichText)},
        {"author", userAtEvent(static_cast<SpectralUser*>(
                                   m_currentRoom->user(replyEvt.senderId())),
                               m_currentRoom, evt)}};
  }

  if (role == ShowAuthorRole) {
    for (auto r = row + 1; r < 0; ++r) {
      auto i = index(r);
      if (data(i, SpecialMarksRole) != EventStatus::Hidden) {
        return data(i, AuthorRole) != data(idx, AuthorRole) ||
               data(i, EventTypeRole) != data(idx, EventTypeRole) ||
               data(idx, TimeRole)
                       .toDateTime()
                       .msecsTo(data(i, TimeRole).toDateTime()) > 600000;
      }
    }

    return true;
  }

  if (role == ShowSectionRole) {
    for (auto r = row + 1; r < rowCount(); ++r) {
      auto i = index(r);
      if (data(i, SpecialMarksRole) != EventStatus::Hidden) {
        return data(i, TimeRole)
                   .toDateTime()
                   .msecsTo(data(idx, TimeRole).toDateTime()) > 600000;
      }
    }

    return true;
  }

  if (role == ReactionRole) {
    const auto& annotations =
        m_currentRoom->relatedEvents(evt, EventRelation::Annotation());
    if (annotations.isEmpty())
      return {};
    QMap<QString, QList<SpectralUser*>> reactions = {};
    for (const auto& a : annotations) {
      if (a->isRedacted())  // Just in case?
        continue;
      if (auto e = eventCast<const ReactionEvent>(a))
        reactions[e->relation().key].append(
            static_cast<SpectralUser*>(m_currentRoom->user(e->senderId())));
    }

    if (reactions.isEmpty()) {
      return {};
    }

    QVariantList res = {};
    auto i = reactions.constBegin();
    while (i != reactions.constEnd()) {
      QVariantList authors;
      for (auto author : i.value()) {
        authors.append(userAtEvent(author, m_currentRoom, evt));
      }
      bool hasLocalUser = i.value().contains(
          static_cast<SpectralUser*>(m_currentRoom->localUser()));
      res.append(QVariantMap{{"reaction", i.key()},
                             {"count", i.value().count()},
                             {"authors", authors},
                             {"hasLocalUser", hasLocalUser}});
      ++i;
    }

    return res;
  }

  return {};
}

int MessageEventModel::eventIDToIndex(const QString& eventID) const {
  const auto it = m_currentRoom->findInTimeline(eventID);
  if (it == m_currentRoom->timelineEdge()) {
    qWarning() << "Trying to find inexistent event:" << eventID;
    return -1;
  }
  return it - m_currentRoom->messageEvents().rbegin() + timelineBaseIndex();
}
