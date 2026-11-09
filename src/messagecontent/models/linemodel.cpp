// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "linemodel.h"

LineModel::LineModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void LineModel::setDocument(QTextDocument *document)
{
    if (document == m_document) {
        return;
    }

    beginResetModel();
    if (m_document) {
        m_document->disconnect(this);
    }
    if (m_documentLayout) {
        m_documentLayout->disconnect(this);
        m_documentLayout = nullptr;
    }
    m_document = document;
    m_blockCount = m_document ? m_document->blockCount() : 0;
    if (m_document) {
        connect(m_document, &QTextDocument::blockCountChanged, this, &LineModel::updateBlockCount);
        connect(m_document, &QTextDocument::documentLayoutChanged, this, &LineModel::connectDocumentLayout);
        connectDocumentLayout();
    }
    Q_EMIT documentChanged();
    endResetModel();
}

void LineModel::connectDocumentLayout()
{
    if (m_documentLayout) {
        m_documentLayout->disconnect(this);
    }
    m_documentLayout = m_document ? m_document->documentLayout() : nullptr;
    if (m_documentLayout) {
        // Line heights change when the text is rewrapped, e.g. because the text item was resized.
        connect(m_documentLayout, &QAbstractTextDocumentLayout::documentSizeChanged, this, &LineModel::scheduleLineHeightUpdate);
    }
    scheduleLineHeightUpdate();
}

void LineModel::updateBlockCount(int blockCount)
{
    if (blockCount == m_blockCount) {
        return;
    }
    beginResetModel();
    m_blockCount = blockCount;
    endResetModel();
}

void LineModel::scheduleLineHeightUpdate()
{
    // The layout notifies us while it is still laying out; querying block geometry from
    // there could re-enter it, so update the heights once it is done.
    if (m_lineHeightUpdatePending) {
        return;
    }
    m_lineHeightUpdatePending = true;
    QMetaObject::invokeMethod(
        this,
        [this]() {
            m_lineHeightUpdatePending = false;
            if (m_blockCount > 0) {
                Q_EMIT dataChanged(index(0), index(m_blockCount - 1), {LineHeightRole});
            }
        },
        Qt::QueuedConnection);
}

void LineModel::setDocument(QQuickTextDocument *document)
{
    setDocument(document->textDocument());
}

QVariant LineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    const auto row = index.row();
    if (row < 0 || row >= m_blockCount || !m_document) {
        return {};
    }

    if (role == LineHeightRole) {
        return int(m_document->documentLayout()->blockBoundingRect(m_document->findBlockByNumber(row)).height());
    }
    return {};
}

int LineModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_blockCount;
}

QHash<int, QByteArray> LineModel::roleNames() const
{
    return {{LineHeightRole, "docLineHeight"}};
}

#include "moc_linemodel.cpp"
