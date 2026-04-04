// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "blockcache.h"

#include <QRegularExpression>

#include "chattextitemhelper.h"
#include "fileinfo.h"

using namespace Blocks;

inline QString formatQuote(const QString &input)
{
    QString stringOut;
    auto splitString = input.split(u"\n\n"_s, Qt::SkipEmptyParts);
    for (auto &string : splitString) {
        if (string.startsWith(u'*')) {
            string.removeFirst();
        }
        if (string.startsWith(u'“')) {
            string.removeFirst();
        }
        if (string.endsWith(u'*')) {
            string.removeLast();
        }
        if (string.endsWith(u'”')) {
            string.removeLast();
        }
        if (!stringOut.isEmpty()) {
            stringOut += u"\n"_s;
        }
        stringOut += u"> "_s + string;
    }
    return stringOut;
}

inline QString trimmedTrailing(QString string)
{
    while (string.endsWith(u' ')) {
        string.removeLast();
    }
    return string;
}

inline QString trimNewline(QString string)
{
    while (string.startsWith(u"\n"_s)) {
        string.removeFirst();
    }
    while (string.endsWith(u"\n"_s)) {
        string.removeLast();
    }
    return string;
}

bool CacheItem::richTextActive = true;

CacheItem::CacheItem(Type type)
    : type(type)
{
}

CacheItem::~CacheItem()
{
}

QString CacheItem::toString() const
{
    return {};
}

TextCacheItem::TextCacheItem(Type type, const QTextDocumentFragment &content, bool hasSpoiler)
    : CacheItem(type)
    , content(content)
    , hasSpoiler(hasSpoiler)
{
}

QString TextCacheItem::toString() const
{
    if (!richTextActive) {
        auto plainText = content.toPlainText();
        const auto markdownText = content.toMarkdown();
        QRegularExpression mentionRegex(u"\\[(.*)]\\(.*\\)"_s);
        QRegularExpressionMatch mentionMatch;
        qsizetype lastPos = 0;
        qsizetype mentionPos = markdownText.indexOf(mentionRegex, lastPos, &mentionMatch);
        while (mentionPos != -1) {
            auto mentionName = mentionMatch.captured(1);
            if (mentionName.startsWith(u"\\"_s)) {
                mentionName.remove(0, 1);
            }
            qsizetype plainPos = plainText.indexOf(mentionName);
            if (plainPos != -1) {
                plainText.replace(plainPos, mentionName.length(), mentionMatch.captured());
            }
            lastPos = mentionPos + mentionMatch.capturedLength();
            mentionPos = markdownText.indexOf(mentionRegex, lastPos, &mentionMatch);
        }
        return trimNewline(plainText);
    }

    QString textOut;
    auto doc = QTextDocument();
    auto cursor = QTextCursor(&doc);
    cursor.insertFragment(content);
    cursor.movePosition(QTextCursor::Start);
    while (!cursor.atEnd()) {
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        auto nextText = trimmedTrailing(trimNewline(cursor.selection().toMarkdown()));
        if (!cursor.currentList()) {
            nextText.replace(u'\n', u' ');
        }
        if (!textOut.isEmpty() && !nextText.isEmpty()) {
            textOut += cursor.currentList() ? u"\n"_s : u"\n\n"_s;
        }
        textOut += nextText;
        cursor.movePosition(QTextCursor::NextBlock);
    }

    textOut = trimNewline(textOut).trimmed();
    return type == Quote ? formatQuote(textOut) : textOut;
}

CodeCacheItem::CodeCacheItem(Type type, const QTextDocumentFragment &content, const QString &language)
    : TextCacheItem(type, content)
    , language(language)
{
}

QString CodeCacheItem::toString() const
{
    const auto trimmedContent = trimNewline(content.toPlainText());
    return u"```%2\n%1\n```"_s.arg(trimmedContent, language).replace(u"\n\n"_s, u"\n"_s);
}

UrlCacheItem::UrlCacheItem(Type type, const QUrl &source)
    : CacheItem(type)
    , source(source)
{
}

FileCacheItem::FileCacheItem(Type type, const QUrl &source, const QString &filename, const FileInfo &info)
    : UrlCacheItem(type, source)
    , filename(filename)
    , info(info)
{
}

ImageCacheItem::ImageCacheItem(Type type,
                               const QUrl &source,
                               const QString &filename,
                               const ImageInfo &info,
                               const QUrl &thumbnailSource,
                               const ImageInfo &thumbnailInfo)
    : UrlCacheItem(type, source)
    , filename(filename)
    , info(info)
    , thumbnailSource(thumbnailSource)
    , thumbnailInfo(thumbnailInfo)
{
}

VideoCacheItem::VideoCacheItem(Type type,
                               const QUrl &source,
                               const QString &filename,
                               const VideoInfo &info,
                               const QUrl &thumbnailSource,
                               const ImageInfo &thumbnailInfo)
    : UrlCacheItem(type, source)
    , filename(filename)
    , info(info)
    , thumbnailSource(thumbnailSource)
    , thumbnailInfo(thumbnailInfo)
{
}

AudioCacheItem::AudioCacheItem(Type type, const QUrl &source, const QString &filename, const AudioInfo &info)
    : UrlCacheItem(type, source)
    , filename(filename)
    , info(info)
{
}

LocationCacheItem::LocationCacheItem(Type type, qreal latitude, qreal longitude, const QString &asset)
    : CacheItem(type)
    , latitude(latitude)
    , longitude(longitude)
    , asset(asset)
{
}

ReplyCacheItem::ReplyCacheItem(Type type, const QString &id)
    : CacheItem(type)
    , id(id)
{
}

Cache::CacheItems::iterator Cache::begin()
{
    return m_items.begin();
}

Cache::CacheItems::iterator Cache::end()
{
    return m_items.end();
}

Cache::CacheItems::const_iterator Cache::begin() const
{
    return m_items.begin();
}

Cache::CacheItems::const_iterator Cache::end() const
{
    return m_items.end();
}

Cache::CacheItems::const_iterator Cache::cbegin() const
{
    return m_items.cbegin();
}

Cache::CacheItems::const_iterator Cache::cend() const
{
    return m_items.cend();
}

bool Cache::empty() const
{
    return m_items.empty();
}

const CacheItem *Cache::at(qsizetype i) const
{
    if (i < 0 || i >= (qsizetype)m_items.size()) {
        return nullptr;
    }
    return m_items.at(i).get();
}

void Cache::prepend(CacheItemPtr item)
{
    m_items.insert(m_items.begin(), std::move(item));
}

void Cache::append(CacheItemPtr item)
{
    m_items.push_back(std::move(item));
}

void Cache::removeAt(qsizetype i)
{
    m_items.erase(m_items.begin() + i);
}

void Cache::clear()
{
    m_items.clear();
}

QString Cache::toString() const
{
    QString text;
    std::ranges::for_each(m_items, [&text](std::unique_ptr<CacheItem> const &item) {
        if (!text.isEmpty()) {
            text += u"\n\n"_s;
        }
        text += item->toString();
    });
    return text;
}

#include "moc_blockcache.cpp"
