#ifndef MESSAGEEVENTMODEL_H
#define MESSAGEEVENTMODEL_H

#pragma once

#include <QtCore/QAbstractListModel>

class MessageEventModel: public QAbstractListModel
{
        Q_OBJECT
        // The below property is marked constant because it only changes
        // when the whole model is reset (so anything that depends on the model
        // has to be re-calculated anyway).
        // XXX: A better way would be to make [Room::]Timeline a list model
        // itself, leaving only representation of the model to a client.
        Q_PROPERTY(QuaternionRoom* room MEMBER m_currentRoom CONSTANT)
    public:
        explicit MessageEventModel(QObject* parent = nullptr);

        void changeRoom(QuaternionRoom* room);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        QHash<int, QByteArray> roleNames() const override;

    private slots:
        void refreshEvent(const QString& eventId);

    private:
        QuaternionRoom* m_currentRoom;
        QString lastReadEventId;

        QDateTime makeMessageTimestamp(QuaternionRoom::rev_iter_t baseIt) const;
        QString makeDateString(QuaternionRoom::rev_iter_t baseIt) const;
        void refreshEventRoles(const QString& eventId, const QVector<int> roles);
};

#endif // MESSAGEEVENTMODEL_H
