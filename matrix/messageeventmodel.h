#ifndef MESSAGEEVENTMODEL_H
#define MESSAGEEVENTMODEL_H

#include <QtCore/QAbstractListModel>
#include "room.h"

class MessageEventModel: public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QMatrixClient::Room* room READ getRoom WRITE setRoom NOTIFY roomChanged)

    public:
        enum EventRoles {
            EventTypeRole = Qt::UserRole + 1,
            EventIdRole,
            TimeRole,
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
            // For debugging
            EventResolvedTypeRole,
        };

        explicit MessageEventModel(QObject* parent = nullptr);
        ~MessageEventModel();

        QMatrixClient::Room* getRoom() { return m_currentRoom; }
        void setRoom(QMatrixClient::Room* room);

        Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        QHash<int, QByteArray> roleNames() const;

    private slots:
        void refreshEvent(const QString& eventId);

    private:
        QMatrixClient::Room* m_currentRoom = nullptr;
        QString lastReadEventId;
        int nextNewerRow = -1;

        QDateTime makeMessageTimestamp(QMatrixClient::Room::rev_iter_t baseIt) const;
        QString makeDateString(QMatrixClient::Room::rev_iter_t baseIt) const;
        void refreshEventRoles(const QString& eventId, const QVector<int> roles);

    signals:
        void roomChanged();
};

#endif // MESSAGEEVENTMODEL_H
