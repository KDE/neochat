#ifndef MATRIQUEROOM_H
#define MATRIQUEROOM_H

#include "room.h"

class MatriqueRoom: public QMatrixClient::Room
{
        Q_OBJECT
    public:
        MatriqueRoom(QMatrixClient::Connection* connection,
                       QString roomId, QMatrixClient::JoinState joinState);

        const QString& cachedInput() const;
        void setCachedInput(const QString& input);

        bool isEventHighlighted(QMatrixClient::RoomEvent* e) const;

        Q_INVOKABLE int savedTopVisibleIndex() const;
        Q_INVOKABLE int savedBottomVisibleIndex() const;
        Q_INVOKABLE void saveViewport(int topIndex, int bottomIndex);

    private slots:
        void countChanged();

    private:
        QSet<QMatrixClient::RoomEvent*> highlights;
        QString m_cachedInput;

        void onAddNewTimelineEvents(timeline_iter_t from) override;
        void onAddHistoricalTimelineEvents(rev_iter_t from) override;

        void checkForHighlights(const QMatrixClient::TimelineItem& ti);
};

#endif // MATRIQUEROOM_H
