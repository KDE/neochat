#ifndef MESSAGEEVENTMODEL_H
#define MESSAGEEVENTMODEL_H

#include <QtCore/QAbstractListModel>

#include "matriqueroom.h"

class MessageEventModel: public QAbstractListModel
{
    Q_OBJECT
    // The below property is marked constant because it only changes
    // when the whole model is reset (so anything that depends on the model
    // has to be re-calculated anyway).
    // XXX: A better way would be to make [Room::]Timeline a list model
    // itself, leaving only representation of the model to a client.
    Q_PROPERTY(MatriqueRoom* room MEMBER m_currentRoom CONSTANT)

    public:
        enum EventRoles {
            EventTypeRole = Qt::UserRole + 1,
            EventIdRole,
            TimeRole,
            SectionRole,
            AboveSectionRole,
            AuthorRole,
            ContentRole,
            ContentTypeRole,
            HighlightRole,
            ReadMarkerRole,
            SpecialMarksRole,
            LongOperationRole,
        };

        explicit MessageEventModel(QObject* parent = nullptr);

        void changeRoom(MatriqueRoom* room);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

    private slots:
        void refreshEvent(const QString& eventId);

    private:
        MatriqueRoom* m_currentRoom;
        QString lastReadEventId;

        QDateTime makeMessageTimestamp(MatriqueRoom::rev_iter_t baseIt) const;
        QString makeDateString(MatriqueRoom::rev_iter_t baseIt) const;
        void refreshEventRoles(const QString& eventId, const QVector<int> roles);
};

#endif // MESSAGEEVENTMODEL_H
