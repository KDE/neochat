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

using namespace Qt::StringLiterals;

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
    QString imageDir(u"%1/screenshots"_s.arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));

    if (!QDir().exists(imageDir)) {
        QDir().mkdir(imageDir);
    }

    if (localPath.isEmpty()) {
        localPath = u"file://%1/%2.png"_s.arg(imageDir, QDateTime::currentDateTime().toString(u"yyyy-MM-dd-hh-mm-ss"_s));
    }
    QUrl url(localPath);
    if (!url.isLocalFile()) {
        return {};
    }
    auto image = this->image();

    if (image.isNull()) {
        return {};
    }

    if (image.save(url.toLocalFile())) {
        return localPath;
    } else {
        return {};
    }
}

void Clipboard::saveText(QString message)
{
    static QRegularExpression re(u"<[^>]*>"_s);
    auto *mimeData = new QMimeData; // ownership is transferred to clipboard
    mimeData->setHtml(message);
    mimeData->setText(message.replace(re, QString()));
    m_clipboard->setMimeData(mimeData);
}

void Clipboard::setImage(const QUrl &url)
{
    if (url.isLocalFile()) {
        QImage img(url.path());
        auto *mimeData = new QMimeData;
        mimeData->setImageData(img);
        if (!img.isNull()) {
            m_clipboard->setMimeData(mimeData);
        }
    }
}

#include "moc_clipboard.cpp"
