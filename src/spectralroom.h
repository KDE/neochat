#ifndef SpectralRoom_H
#define SpectralRoom_H

#include "room.h"
#include "spectraluser.h"

#include <QObject>
#include <QPointer>
#include <QTimer>

#include <events/redactionevent.h>
#include <events/roomavatarevent.h>
#include <events/roomcreateevent.h>
#include <events/roommemberevent.h>
#include <events/roommessageevent.h>
#include <events/simplestateevents.h>

using namespace QMatrixClient;

class SpectralRoom : public Room {
  Q_OBJECT
  Q_PROPERTY(bool hasUsersTyping READ hasUsersTyping NOTIFY typingChanged)
  Q_PROPERTY(QVariantList usersTyping READ getUsersTyping NOTIFY typingChanged)
  Q_PROPERTY(QString cachedInput READ cachedInput WRITE setCachedInput NOTIFY
                 cachedInputChanged)
  Q_PROPERTY(bool hasFileUploading READ hasFileUploading NOTIFY
                 hasFileUploadingChanged)
  Q_PROPERTY(int fileUploadingProgress READ fileUploadingProgress NOTIFY
                 fileUploadingProgressChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

 public:
  explicit SpectralRoom(Connection* connection,
                        QString roomId,
                        JoinState joinState = {});

  const QString& cachedInput() const { return m_cachedInput; }
  void setCachedInput(const QString& input) {
    if (input != m_cachedInput) {
      m_cachedInput = input;
      emit cachedInputChanged();
    }
  }

  bool busy() { return m_busy; }
  void setBusy(bool value) {
    if (m_busy != value) {
      m_busy = value;
      emit busyChanged();
    }
  }

  bool hasUsersTyping();
  QVariantList getUsersTyping();

  QString lastEvent();
  bool isEventHighlighted(const QMatrixClient::RoomEvent* e) const;

  QDateTime lastActiveTime();

  bool hasFileUploading() { return m_hasFileUploading; }
  void setHasFileUploading(bool value) {
    if (m_hasFileUploading != value) {
      m_hasFileUploading = value;
      emit hasFileUploadingChanged();
    }
  }

  int fileUploadingProgress() { return m_fileUploadingProgress; }
  void setFileUploadingProgress(int value) {
    if (m_fileUploadingProgress != value) {
      m_fileUploadingProgress = value;
      emit fileUploadingProgressChanged();
    }
  }

  Q_INVOKABLE int savedTopVisibleIndex() const;
  Q_INVOKABLE int savedBottomVisibleIndex() const;
  Q_INVOKABLE void saveViewport(int topIndex, int bottomIndex);

  Q_INVOKABLE void getPreviousContent(int limit = 10);

  Q_INVOKABLE QVariantList getUsers(const QString& prefix);

  Q_INVOKABLE QString postMarkdownText(const QString& markdown);

  template <typename BaseEventT>
  QString eventToString(const BaseEventT& evt,
                        Qt::TextFormat format = Qt::PlainText) {
    bool prettyPrint = (format == Qt::RichText);

    using namespace QMatrixClient;
    return visit(
        evt,
        [this, prettyPrint](const RoomMessageEvent& e) {
          using namespace MessageEventContent;

          if (prettyPrint && e.hasTextContent() &&
              e.mimeType().name() != "text/plain")
            return static_cast<const TextContent*>(e.content())->body;
          if (e.hasFileContent()) {
            auto fileCaption =
                e.content()->fileInfo()->originalName.toHtmlEscaped();
            if (fileCaption.isEmpty()) {
              if (prettyPrint)
                fileCaption = this->prettyPrint(e.plainBody());
              else
                fileCaption = e.plainBody();
            }
            return !fileCaption.isEmpty() ? fileCaption : tr("a file");
          }
          return prettyPrint ? this->prettyPrint(e.plainBody()) : e.plainBody();
        },
        [this](const RoomMemberEvent& e) {
          // FIXME: Rewind to the name that was at the time of this event
          auto subjectName = this->user(e.userId())->displayname();
          // The below code assumes senderName output in AuthorRole
          switch (e.membership()) {
            case MembershipType::Invite:
              if (e.repeatsState())
                return tr("reinvited %1 to the room").arg(subjectName);
              FALLTHROUGH;
            case MembershipType::Join: {
              if (e.repeatsState())
                return tr("joined the room (repeated)");
              if (!e.prevContent() ||
                  e.membership() != e.prevContent()->membership) {
                return e.membership() == MembershipType::Invite
                           ? tr("invited %1 to the room").arg(subjectName)
                           : tr("joined the room");
              }
              QString text{};
              if (e.isRename()) {
                if (e.displayName().isEmpty())
                  text = tr("cleared the display name");
                else
                  text = tr("changed the display name to %1")
                             .arg(e.displayName().toHtmlEscaped());
              }
              if (e.isAvatarUpdate()) {
                if (!text.isEmpty())
                  text += " and ";
                if (e.avatarUrl().isEmpty())
                  text += tr("cleared the avatar");
                else
                  text += tr("updated the avatar");
              }
              return text;
            }
            case MembershipType::Leave:
              if (e.prevContent() &&
                  e.prevContent()->membership == MembershipType::Invite) {
                return (e.senderId() != e.userId())
                           ? tr("withdrew %1's invitation").arg(subjectName)
                           : tr("rejected the invitation");
              }

              if (e.prevContent() &&
                  e.prevContent()->membership == MembershipType::Ban) {
                return (e.senderId() != e.userId())
                           ? tr("unbanned %1").arg(subjectName)
                           : tr("self-unbanned");
              }
              return (e.senderId() != e.userId())
                         ? tr("has put %1 out of the room: %2")
                               .arg(subjectName, e.contentJson()["reason"_ls]
                                                     .toString()
                                                     .toHtmlEscaped())
                         : tr("left the room");
            case MembershipType::Ban:
              return (e.senderId() != e.userId())
                         ? tr("banned %1 from the room: %2")
                               .arg(subjectName, e.contentJson()["reason"_ls]
                                                     .toString()
                                                     .toHtmlEscaped())
                         : tr("self-banned from the room");
            case MembershipType::Knock:
              return tr("knocked");
            default:;
          }
          return tr("made something unknown");
        },
        [](const RoomAliasesEvent& e) {
          return tr("has set room aliases on server %1 to: %2")
              .arg(e.stateKey(), QLocale().createSeparatedList(e.aliases()));
        },
        [](const RoomCanonicalAliasEvent& e) {
          return (e.alias().isEmpty())
                     ? tr("cleared the room main alias")
                     : tr("set the room main alias to: %1").arg(e.alias());
        },
        [](const RoomNameEvent& e) {
          return (e.name().isEmpty()) ? tr("cleared the room name")
                                      : tr("set the room name to: %1")
                                            .arg(e.name().toHtmlEscaped());
        },
        [this, prettyPrint](const RoomTopicEvent& e) {
          return (e.topic().isEmpty())
                     ? tr("cleared the topic")
                     : tr("set the topic to: %1")
                           .arg(prettyPrint ? this->prettyPrint(e.topic())
                                            : e.topic());
        },
        [](const RoomAvatarEvent&) { return tr("changed the room avatar"); },
        [](const EncryptionEvent&) {
          return tr("activated End-to-End Encryption");
        },
        [](const RoomCreateEvent& e) {
          return (e.isUpgrade() ? tr("upgraded the room to version %1")
                                : tr("created the room, version %1"))
              .arg(e.version().isEmpty() ? "1" : e.version().toHtmlEscaped());
        },
        [](const StateEventBase& e) {
          // A small hack for state events from TWIM bot
          return e.stateKey() == "twim"
                     ? tr("updated the database",
                          "TWIM bot updated the database")
                     : e.stateKey().isEmpty()
                           ? tr("updated %1 state", "%1 - Matrix event type")
                                 .arg(e.matrixType())
                           : tr("updated %1 state for %2",
                                "%1 - Matrix event type, %2 - state key")
                                 .arg(e.matrixType(),
                                      e.stateKey().toHtmlEscaped());
        },
        tr("Unknown event"));
  }

 private:
  QString m_cachedInput;
  QSet<const QMatrixClient::RoomEvent*> highlights;

  bool m_hasFileUploading = false;
  int m_fileUploadingProgress = 0;

  bool m_busy = false;

  void checkForHighlights(const QMatrixClient::TimelineItem& ti);

  void onAddNewTimelineEvents(timeline_iter_t from) override;
  void onAddHistoricalTimelineEvents(rev_iter_t from) override;

 private slots:
  void countChanged();

 signals:
  void cachedInputChanged();
  void busyChanged();
  void hasFileUploadingChanged();
  void fileUploadingProgressChanged();

 public slots:
  void chooseAndUploadFile();
  void acceptInvitation();
  void forget();
  void sendTypingNotification(bool isTyping);
  void sendReply(QString userId,
                 QString eventId,
                 QString replyContent,
                 QString sendContent);
};

#endif  // SpectralRoom_H
