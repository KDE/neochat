// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QUrl>

#include "block.h"

namespace Blocks
{
class FilePreviewBlockLoader : public QObject
{
    Q_OBJECT

public:
    FilePreviewBlockLoader(QObject *parent, const QUrl &source);

    Block *previewBlock();

    bool loaded() const;

Q_SIGNALS:
    void blockAvailable();
    void blockUnavailable();

private:
    QUrl m_source;

    Block *m_previewBlock;
    void blockForFile();

    bool m_loaded = false;
};
}
