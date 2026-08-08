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

#include "texthandler.h"

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

bool Clipboard::hasUriList() const
{
    return !uriList().isEmpty();
}

QList<QUrl> Clipboard::uriList() const
{
    const auto mimeData = m_clipboard->mimeData();
    if (mimeData->hasUrls()) {
        return mimeData->urls();
    }

    return {};
}

QUrl Clipboard::saveImage() const
{
    QString imageDir(u"%1/screenshots"_s.arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)));

    if (!QDir().exists(imageDir)) {
        QDir().mkdir(imageDir);
    }

    QUrl url(u"file://%1/%2.png"_s.arg(imageDir, QDateTime::currentDateTime().toString(u"yyyy-MM-dd-hh-mm-ss"_s)));
    if (!url.isLocalFile()) {
        return {};
    }
    auto image = this->image();

    if (image.isNull()) {
        return {};
    }

    if (image.save(url.toLocalFile())) {
        return url;
    }
    return {};
}

QString Clipboard::plainText() const
{
    const auto mimeData = m_clipboard->mimeData();
    if (mimeData->hasHtml()) {
        return QTextDocumentFragment::fromHtml(mimeData->html()).toPlainText();
    } else {
        return mimeData->text();
    }
}

QString Clipboard::richText(PasteMode mode) const
{
    const auto mimeData = m_clipboard->mimeData();
    if (mimeData->hasHtml()) {
        return TextHandler::cleanHtml(mimeData->html());
    } else if (mimeData->hasText() && mode == PasteMode::ConvertMarkdown) {
        auto richText = mimeData->text();
        TextHandler::markdownToHtml(richText);
        if (richText.count("<p>"_L1) == 1 && richText.count("</p>"_L1) == 1 && richText.startsWith("<p>"_L1) && richText.endsWith("</p>"_L1)) {
            richText.remove("<p>"_L1);
            richText.remove("</p>"_L1);
        }
        return richText;
    } else if (mimeData->hasText() && mode == PasteMode::PlainToRich) {
        auto text = mimeData->text();
        text.replace(u"\n"_s, u"<br>"_s);
        return text;
    }
    return {};
}

void Clipboard::saveText(QString message)
{
    static QRegularExpression re(u"<[^>]*>"_s);
    auto *mimeData = new QMimeData; // ownership is transferred to clipboard
    mimeData->setHtml(message);
    mimeData->setText(message.replace(re, QString()));
    m_clipboard->setMimeData(mimeData);
}

void Clipboard::setMimeData(QMimeData *mimeData)
{
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
