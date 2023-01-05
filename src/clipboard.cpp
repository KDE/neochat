// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

#include "clipboard.h"

#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QMimeData>
#include <QRegularExpression>
#include <QStandardPaths>
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

QString Clipboard::saveImage(QString localPath) const
{
    if (!QDir().exists(QStringLiteral("%1/screenshots").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)))) {
        QDir().mkdir(QStringLiteral("%1/screenshots").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));
    }
    if (localPath.isEmpty()) {
        localPath = QStringLiteral("file://%1/screenshots/%2.png")
                        .arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation),
                             QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd-hh-mm-ss")));
    }
    QUrl url(localPath);
    if (!url.isLocalFile()) {
        return {};
    }
    auto image = this->image();

    if (image.isNull()) {
        return {};
    }

    QDir dir;
    if (!dir.exists(QFileInfo(url.fileName()).absoluteFilePath())) {
        dir.mkpath(QFileInfo(url.fileName()).absoluteFilePath());
    }

    image.save(url.toLocalFile());

    return localPath;
}

void Clipboard::saveText(QString message)
{
    QRegularExpression re("<[^>]*>");
    auto *mineData = new QMimeData; // ownership is transferred to clipboard
    mineData->setHtml(message);
    mineData->setText(message.replace(re, ""));
    m_clipboard->setMimeData(mineData);
}
