// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "stickerlist.h"

#include <QVariant>

#include "fileinfo.h"
#include "stickermodel.h"

using namespace Qt::StringLiterals;

StickerList::StickerList(const Quotient::ImagePackEventContent &imagePackContent, std::function<QUrl(QUrl)> makeMediaUrlFunction, QObject *parent)
    : QObject(parent)
    , m_imagePackContent(imagePackContent)
    , m_makeMediaUrlFunction(makeMediaUrlFunction)
{
}

QString StickerList::name() const
{
    const auto pack = m_imagePackContent.pack;
    return pack && pack->displayName ? *pack->displayName : u""_s;
}

QUrl StickerList::source() const
{
    const auto pack = m_imagePackContent.pack;
    const auto source = pack && pack->avatarUrl ? *pack->avatarUrl : (m_imagePackContent.images.isEmpty() ? QUrl() : m_imagePackContent.images[0].url);
    return m_makeMediaUrlFunction && !source.isEmpty() ? m_makeMediaUrlFunction(source) : source;
}

qsizetype StickerList::size() const
{
    return m_imagePackContent.images.size();
}

QVariant StickerList::data(qsizetype row, int role) const
{
    if (row < 0 || row >= m_imagePackContent.images.count()) {
        return {};
    }
    if (role == StickerModel::SourceRole) {
        const auto source = m_imagePackContent.images[row].url;
        return source.isEmpty() ? source : m_makeMediaUrlFunction(source);
    }
    if (role == StickerModel::NameRole) {
        const auto body = m_imagePackContent.images[row].body;
        return u"%1%2"_s.arg(m_imagePackContent.images[row].shortcode, body ? u" %1"_s.arg(*body) : u""_s);
    }
    if (role == StickerModel::ShortCodeRole) {
        return m_imagePackContent.images[row].shortcode;
    }
    if (role == StickerModel::DescriptionRole) {
        return m_imagePackContent.images[row].body ? *m_imagePackContent.images[row].body : QString();
    }
    if (role == StickerModel::PixelSizeRole) {
        return m_imagePackContent.images[row].info ? m_imagePackContent.images[row].info->imageSize : QSize();
    }
    if (role == StickerModel::SizeRole) {
        return m_imagePackContent.images[row].info ? m_imagePackContent.images[row].info->payloadSize : 0;
    }
    if (role == StickerModel::MimeRole) {
        return QVariant::fromValue(m_imagePackContent.images[row].info ? m_imagePackContent.images[row].info->mimeType : QMimeType());
    }
    return {};
}

#include "moc_stickerlist.cpp"
