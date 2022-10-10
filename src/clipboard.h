// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QObject>

class QClipboard;
class QImage;

/**
 * Clipboard proxy
 */
class Clipboard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasImage READ hasImage NOTIFY imageChanged)
    Q_PROPERTY(QImage image READ image NOTIFY imageChanged)

public:
    explicit Clipboard(QObject *parent = nullptr);

    [[nodiscard]] bool hasImage() const;
    [[nodiscard]] QImage image() const;

    Q_INVOKABLE QString saveImage(QString localPath = {}) const;

    Q_INVOKABLE void saveText(QString message);

private:
    QClipboard *m_clipboard;

Q_SIGNALS:
    void imageChanged();
};
