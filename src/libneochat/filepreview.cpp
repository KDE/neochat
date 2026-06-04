// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "filepreview.h"

#include <QImageReader>

#ifndef Q_OS_ANDROID
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Repository>
#endif

#include "fileinfo.h"
#include "filetype.h"

using namespace Blocks;

FilePreviewBlockLoader::FilePreviewBlockLoader(QObject *parent, const QUrl &source)
    : QObject(parent)
    , m_source(source)
{
    Q_ASSERT(parent);
    if (!m_source.isLocalFile()) {
        m_state = Unavailable;
        Q_EMIT blockUnavailable();
        return;
    }

    m_state = Loading;
    const auto block = new Blocks::ItineraryBlock(Blocks::Itinerary, m_source, parent);
    m_previewBlock = block;
    connect(block->model(), &ItineraryModel::loaded, this, [this, block]() {
        if (block->model()->rowCount() > 0) {
            m_state = Available;
            Q_EMIT blockAvailable();
            return;
        }
        block->deleteLater();
        m_previewBlock = nullptr;
        blockForFile();
    });
    connect(block->model(), &ItineraryModel::loadErrorOccurred, this, [this, block]() {
        block->deleteLater();
        m_previewBlock = nullptr;
        blockForFile();
    });
}

void FilePreviewBlockLoader::blockForFile()
{
#ifndef Q_OS_ANDROID
    const QMimeType mimeType = FileType::instance().mimeTypeForFile(m_source.toString());
    if (mimeType.inherits(u"text/plain"_s)) {
        KSyntaxHighlighting::Repository repository;
        KSyntaxHighlighting::Definition definitionForFile = repository.definitionForFileName(m_source.toString());
        if (!definitionForFile.isValid()) {
            definitionForFile = repository.definitionForMimeType(mimeType.name());
        }

        QFile file(m_source.path());
        auto ok = file.open(QIODevice::ReadOnly);
        if (!ok) {
            qWarning() << "Failed to open" << m_source.path() << file.errorString();
        }

        m_previewBlock = new Blocks::CodeBlock(Blocks::Code,
                                               QTextDocumentFragment::fromPlainText(QString::fromStdString(file.readAll().toStdString())),
                                               definitionForFile.name(),
                                               parent());
        m_state = Available;
        Q_EMIT blockAvailable();
        return;
    }
#endif

    if (FileType::instance().fileHasImage(m_source)) {
        QImageReader reader(m_source.path());
        Blocks::ImageInfo info;
        info.pixelSize = reader.size();
        m_previewBlock = new Blocks::ImageBlock(Blocks::Pdf, m_source, m_source.fileName(), info, QUrl(), Blocks::ImageInfo(), parent());
        m_state = Available;
        Q_EMIT blockAvailable();
        return;
    }

    m_state = Unavailable;
    Q_EMIT blockUnavailable();
}

Block *FilePreviewBlockLoader::previewBlock()
{
    return m_previewBlock;
}

FilePreviewBlockLoader::State FilePreviewBlockLoader::state() const
{
    return m_state;
}

#include "moc_filepreview.cpp"
