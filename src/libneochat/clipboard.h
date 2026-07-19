// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>
#include <QQmlEngine>

class QClipboard;
class QImage;

/**
 * @class Clipboard
 *
 * Clipboard proxy
 *
 * Used to set and retrieve content from the clipboard.
 */
class Clipboard : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    /**
     * @brief Whether the current clipboard content is an image.
     */
    Q_PROPERTY(bool hasImage READ hasImage NOTIFY imageChanged)

    /**
     * @brief Return the current clipboard content image.
     *
     * Returns a null image if the clipboard does not contain an image or if it
     * contains an image in an unsupported image format.
     */
    Q_PROPERTY(QImage image READ image NOTIFY imageChanged)

public:
    enum PasteMode {
        Raw,
        PlainToRich,
        ConvertMarkdown,
    };
    Q_ENUM(PasteMode)

    explicit Clipboard(QObject *parent = nullptr);

    [[nodiscard]] bool hasImage() const;

    [[nodiscard]] QImage image() const;

    [[nodiscard]] bool hasUriList() const;

    [[nodiscard]] QList<QUrl> uriList() const;

    /**
     * @brief Save the current clipboard image to file.
     *
     * If the clipboard does not contain an image or if it contains an image in an
     * unsupported image format nothing happens.
     *
     * @return A QString with the path that the image was saved to. The string will
     *         be empty if nothing was saved.
     */
    Q_INVOKABLE QUrl saveImage() const;

    /**
     * @brief Return the current clipboard text as plain text.
     *
     * Returns an empty string if the clipboard is empty or has no text.
     *
     * If the current clipboard contents are rich text they will be converted to
     * plain text.
     */
    Q_INVOKABLE [[nodiscard]] QString plainText() const;

    /**
     * @brief Return the current clipboard text as rich text.
     *
     * Returns an empty string if the clipboard is empty or has no text.
     *
     * If clipboard contains plain text with markdown formatting it will be converted
     * to rich text.
     */
    Q_INVOKABLE [[nodiscard]] QString richText(PasteMode mode) const;

    /**
     * @brief Set the clipboard content to the input message.
     */
    Q_INVOKABLE void saveText(QString message);

    /**
     * @brief Set the clipboard content to the input image.
     */
    Q_INVOKABLE void setImage(const QUrl &image);

private:
    QClipboard *m_clipboard;

Q_SIGNALS:
    void imageChanged();
};
