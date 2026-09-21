// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "completionproxymodel.h"

#include <Kirigami/Platform/PlatformTheme>

#include "completionmodel.h"

using namespace Qt::StringLiterals;

ChatTextItemHelper *CompletionProxyModel::textItem() const
{
    return m_textItem;
}

void CompletionProxyModel::setTextItem(ChatTextItemHelper *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    if (m_textItem) {
        m_textItem->disconnect(this);
    }

    m_textItem = textItem;

    if (m_textItem) {
        connect(m_textItem, &ChatTextItemHelper::cursorPositionChanged, this, &CompletionProxyModel::updateFilterText);
        connect(m_textItem, &ChatTextItemHelper::contentsChanged, this, &CompletionProxyModel::updateFilterText);
    }
    Q_EMIT textItemChanged();
}

void CompletionProxyModel::updateFilterText()
{
    auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }

    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    while (cursor.selectedText() != u' ' && !cursor.atBlockStart()) {
        cursor.movePosition(QTextCursor::PreviousCharacter);
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }
    if (cursor.position() != 0 || cursor.selectedText() == u' ') {
        cursor.movePosition(QTextCursor::NextCharacter);
    } else {
        cursor.setPosition(cursor.position());
    }

    if (m_ignoreCurrentCompletion) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == u' ') {
            m_ignoreCurrentCompletion = false;
        }
        return;
    }

    while (!cursor.selectedText().endsWith(u' ') && !cursor.atBlockEnd()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    const auto newFilterText = cursor.selectedText().trimmed();
    if (newFilterText != m_filterText) {
        beginFilterChange();
        m_filterText = newFilterText;
        if (const auto completionModel = dynamic_cast<CompletionModel *>(sourceModel())) {
            completionModel->setCurrentText(newFilterText);
        }
        endFilterChange();
        const bool isCompleting = rowCount() > 0;
        if (m_textItem->isCompleting != isCompleting) {
            m_textItem->isCompleting = isCompleting;
            Q_EMIT isCompletingChanged();
        }
    }
}

bool CompletionProxyModel::isCompleting() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->isCompleting;
}

void CompletionProxyModel::ignoreCurrentCompletion()
{
    m_ignoreCurrentCompletion = true;

    if (m_textItem->isCompleting) {
        m_textItem->isCompleting = false;
        Q_EMIT isCompletingChanged();
    }
}

void CompletionProxyModel::insertCompletion(const QString &text, const QUrl &link)
{
    if (!m_textItem->isCompleting) {
        return;
    }
    QTextCursor cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }

    cursor.beginEditBlock();
    while (!cursor.selectedText().startsWith(u' ') && !cursor.atBlockStart()) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
    }
    if (cursor.selectedText().startsWith(u' ')) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    cursor.removeSelectedText();

    const auto previousFormat = cursor.charFormat();
    auto charFormat = previousFormat;
    if (link.isValid()) {
        const auto theme = static_cast<Kirigami::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<Kirigami::Platform::PlatformTheme>(this, true));
        charFormat = QTextCharFormat();
        charFormat.setForeground(theme->linkColor());
        charFormat.setFontWeight(QFont::Bold);
        charFormat.setAnchor(true);
        charFormat.setAnchorHref(link.toString());
    }
    cursor.insertText(text, charFormat);
    if (!link.isEmpty()) {
        cursor.insertText(u" "_s, previousFormat);
    }
    cursor.endEditBlock();
    m_textItem->isCompleting = false;
    updateFilterText();
}

bool CompletionProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    if (m_filterText.isEmpty()) {
        return false;
    }

    const auto sourceIndex = sourceModel()->index(sourceRow, 0);
    const auto startSequence = sourceIndex.data(CompletionModel::StartSequenceRole).toString();
    const auto noStartFilterText = m_filterText.last(m_filterText.size() - startSequence.size());
    const auto matchSequences = sourceIndex.data(CompletionModel::MatchSequencesRole).toStringList();
    return !matchSequences.isEmpty() && m_filterText.startsWith(startSequence)
        && std::ranges::any_of(matchSequences, [noStartFilterText](const QString &matchSequence) {
               return matchSequence.startsWith(noStartFilterText);
           });
}

#include "moc_completionproxymodel.cpp"
