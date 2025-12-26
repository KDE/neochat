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

    m_document = document;
    Q_EMIT documentChanged();

    resetModel();
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

    const auto &row = index.row();
    if (row < 0 || row > rowCount()) {
        return {};
    }

    if (role == LineHeightRole) {
        return int(m_document->documentLayout()->blockBoundingRect(m_document->findBlockByNumber(row)).height());
    }
    return {};
}

int LineModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_document == nullptr) {
        return 0;
    }
    return m_document->blockCount();
}

QHash<int, QByteArray> LineModel::roleNames() const
{
    return {{LineHeightRole, "docLineHeight"}};
}

void LineModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

#include "moc_linemodel.cpp"
