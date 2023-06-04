// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QSortFilterProxyModel>

/**
 * @class EmoticonFilterModel
 *
 * This class creates a custom QSortFilterProxyModel for filtering a emoticon by type
 * (Sticker or Emoji).
 */
class EmoticonFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    /**
     * @brief Whether stickers should be shown
     */
    Q_PROPERTY(bool showStickers READ showStickers WRITE setShowStickers NOTIFY showStickersChanged)

    /**
     * @brief Whether emojis show be shown
     */
    Q_PROPERTY(bool showEmojis READ showEmojis WRITE setShowEmojis NOTIFY showEmojisChanged)

public:
    explicit EmoticonFilterModel(QObject *parent = nullptr);

    /**
     * @brief Custom filter function checking the type of emoticon
     *
     * @note The filter cannot be modified and will always use the same filter properties.
     */
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    [[nodiscard]] bool showStickers() const;
    void setShowStickers(bool showStickers);

    [[nodiscard]] bool showEmojis() const;
    void setShowEmojis(bool showEmojis);

Q_SIGNALS:
    void showStickersChanged();
    void showEmojisChanged();

private:
    bool m_showStickers = false;
    bool m_showEmojis = false;
    int m_stickerRole = 0;
    int m_emojiRole = 0;
};
