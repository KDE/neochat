#ifndef MESSAGEEVENTMODEL_H
#define MESSAGEEVENTMODEL_H

#include <QtCore/QAbstractListModel>

#include "room.h"
#include "spectralroom.h"

class MessageEventModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(SpectralRoom* room READ room WRITE setRoom NOTIFY roomChanged)

 public:
  enum EventRoles {
    EventTypeRole = Qt::UserRole + 1,
    MessageRole,
    EventIdRole,
    TimeRole,
    SectionRole,
    AuthorRole,
    ContentRole,
    ContentTypeRole,
    HighlightRole,
    ReadMarkerRole,
    SpecialMarksRole,
    LongOperationRole,
    AnnotationRole,
    UserMarkerRole,

    ReplyRole,

    ShowAuthorRole,
    ShowSectionRole,

    BubbleShapeRole,

    ReactionRole,

    // For debugging
    EventResolvedTypeRole,
  };

  enum BubbleShapes {
    NoShape = 0,
    BeginShape,
    MiddleShape,
    EndShape,
  };

  explicit MessageEventModel(QObject* parent = nullptr);
  ~MessageEventModel() override;

  SpectralRoom* room() const { return m_currentRoom; }
  void setRoom(SpectralRoom* room);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE int eventIDToIndex(const QString& eventID) const;

 private slots:
  int refreshEvent(const QString& eventId);
  void refreshRow(int row);

 private:
  SpectralRoom* m_currentRoom = nullptr;
  QString lastReadEventId;
  int rowBelowInserted = -1;
  bool movingEvent = 0;

  int timelineBaseIndex() const;
  QDateTime makeMessageTimestamp(
      const Quotient::Room::rev_iter_t& baseIt) const;
  QString renderDate(QDateTime timestamp) const;

  void refreshLastUserEvents(int baseRow);
  void refreshEventRoles(int row, const QVector<int>& roles = {});
  int refreshEventRoles(const QString& eventId, const QVector<int>& roles = {});

 signals:
  void roomChanged();
};

#endif  // MESSAGEEVENTMODEL_H
