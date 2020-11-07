/**
 * SPDX-FileCopyrightText: Black Hat <bhat@encom.eu.org>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
#include "clipboard.h"

#include <QClipboard>
#include <QMimeData>
#include <QRegularExpression>
#include <QImage>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QUrl>

Clipboard::Clipboard(QObject *parent)
    : QObject(parent)
    , m_clipboard(QGuiApplication::clipboard())
{
    connect(m_clipboard, &QClipboard::changed, this, &Clipboard::imageChanged);
}

bool Clipboard::hasImage() const
{
    return !image().isNull();
}

QImage Clipboard::image() const
{
    return m_clipboard->image();
}

bool Clipboard::saveImage(const QUrl &localPath)
{
    if (!localPath.isLocalFile())
        return false;

    auto i = image();

    if (i.isNull())
        return false;

    QString path = QFileInfo(localPath.toLocalFile()).absolutePath();
    QDir dir;
    if (!dir.exists(path)) {
        dir.mkpath(path);
    }

    i.save(localPath.toLocalFile());

    return true;
}

void Clipboard::saveText(QString message)
{
    QRegularExpression re("<[^>]*>");
    auto *mineData = new QMimeData; // ownership is transfered to clipboard
    mineData->setHtml(message);
    mineData->setText(message.replace(re, ""));
    m_clipboard->setMimeData(mineData);
}
