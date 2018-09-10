#ifndef MESSAGEEVENTMODEL_H
#define MESSAGEEVENTMODEL_H

#include "matriqueroom.h"
#include "room.h"

#include <QtCore/QAbstractListModel>

class MessageEventModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(MatriqueRoom* room READ getRoom WRITE setRoom NOTIFY roomChanged)

 public:
  enum EventRoles {
    EventTypeRole = Qt::UserRole + 1,
    AboveEventTypeRole,
    EventIdRole,
    TimeRole,
    AboveTimeRole,
    SectionRole,
    AboveSectionRole,
    AuthorRole,
    AboveAuthorRole,
    ContentRole,
    ContentTypeRole,
    HighlightRole,
    ReadMarkerRole,
    SpecialMarksRole,
    LongOperationRole,
    AnnotationRole,
    PlainTextRole,
    // For debugging
    EventResolvedTypeRole,
  };

  explicit MessageEventModel(QObject* parent = nullptr);
  ~MessageEventModel();

  MatriqueRoom* getRoom() { return m_currentRoom; }
  void setRoom(MatriqueRoom* room);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const;

 private slots:
  int refreshEvent(const QString& eventId);
  void refreshRow(int row);

 private:
  MatriqueRoom* m_currentRoom = nullptr;
  QString lastReadEventId;
  int rowBelowInserted = -1;
  bool movingEvent = 0;

  int timelineBaseIndex() const;
  QDateTime makeMessageTimestamp(
      const QMatrixClient::Room::rev_iter_t& baseIt) const;
  QString renderDate(QDateTime timestamp) const;
  bool isUserActivityNotable(
      const QMatrixClient::Room::rev_iter_t& baseIt) const;

  void refreshLastUserEvents(int baseRow);
  void refreshEventRoles(int row, const QVector<int>& roles = {});
  int refreshEventRoles(const QString& eventId, const QVector<int>& roles = {});

 signals:
  void roomChanged();
};

#endif  // MESSAGEEVENTMODEL_H
