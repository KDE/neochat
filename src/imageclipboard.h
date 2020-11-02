/**
 * SPDX-FileCopyrightText: Black Hat <bhat@encom.eu.org>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
#ifndef IMAGECLIPBOARD_H
#define IMAGECLIPBOARD_H

#include <QClipboard>
#include <QImage>
#include <QObject>

class ImageClipboard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasImage READ hasImage NOTIFY imageChanged)
    Q_PROPERTY(QImage image READ image NOTIFY imageChanged)

public:
    explicit ImageClipboard(QObject *parent = nullptr);

    bool hasImage() const;
    QImage image() const;

    Q_INVOKABLE bool saveImage(const QUrl &localPath);

private:
    QClipboard *m_clipboard;

signals:
    void imageChanged();
};

#endif // IMAGECLIPBOARD_H
