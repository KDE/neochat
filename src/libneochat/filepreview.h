// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QUrl>

#include "block.h"

namespace Blocks
{
/**
 * @class FilePreviewBlockLoader
 *
 * Load a Blocks::Block to preview a file at the given source if available.
 *
 * The source must be a local file and must be a file type that the loader understands.
 *
 * The FilePreviewBlockLoader must be parented to the QObject that intends to own the
 * file preview block as FilePreviewBlockLoader will set the parent of the preview to it.
 *
 * The current list of supported files are:
 *  - Text (or anything that can be opened as such)
 *  - Pdfs
 *  - Itinerary - anything that KItinerary's extractor can produce a model for
 */
class FilePreviewBlockLoader : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Defines the state of the loader.
     */
    enum State {
        NotStarted, /**< The loader has not started. */
        Loading, /**< The loader is currently trying to load a block. */
        Available, /**< The loader has successfully loaded a block and it's ready. */
        Unavailable, /**< The loader has failed to load a block for the given source. */
    };

    FilePreviewBlockLoader(QObject *parent, const QUrl &source);

    /**
     * @brief The block to preview the file.
     *
     * The block will be parented to the parent of this FilePreviewBlockLoader.
     */
    Block *previewBlock();

    /**
     * @brief The current FilePreviewBlockLoader::State of the loader.
     */
    State state() const;

Q_SIGNALS:
    /**
     * @brief Emitted when the loader has successfully loaded a block that is ready to use.
     */
    void blockAvailable();

    /**
     * @brief Emitted when a preview block cannot be created for the given source.
     */
    void blockUnavailable();

private:
    QUrl m_source;

    Block *m_previewBlock = nullptr;
    void blockForFile();

    State m_state = NotStarted;
};
}
