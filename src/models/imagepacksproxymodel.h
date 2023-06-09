// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QSortFilterProxyModel>
#include <QQmlEngine>

class NeoChatRoom;

/**
 * Filters image packs on whether they contain stickers or emojis, depending on the respective properties
 */
class ImagePacksProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool stickers READ stickers WRITE setStickers NOTIFY stickersChanged)
    Q_PROPERTY(bool emojis READ emojis WRITE setEmojis NOTIFY emojisChanged)
    Q_PROPERTY(NeoChatRoom *currentRoom READ currentRoom WRITE setCurrentRoom NOTIFY currentRoomChanged)

public:
    explicit ImagePacksProxyModel(QObject *parent = nullptr);

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    [[nodiscard]] bool stickers() const;
    void setStickers(bool stickers);

    [[nodiscard]] bool emojis() const;
    void setEmojis(bool emojis);

    [[nodiscard]] NeoChatRoom *currentRoom() const;
    void setCurrentRoom(NeoChatRoom *currentRoom);

Q_SIGNALS:
    void stickersChanged();
    void emojisChanged();
    void currentRoomChanged();

private:
    bool m_stickers = true;
    bool m_emojis = true;
};
