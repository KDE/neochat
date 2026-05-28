// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "completionmodel.h"

#include <QDebug>
#include <QTextCursor>

#include <Kirigami/Platform/PlatformTheme>

#include "userlistmodel.h"

class UserCompletionProvider : public CompletionProvider
{
public:
    UserCompletionProvider(QObject *parent)
        : CompletionProvider(parent)
    {
    }
    bool matchesPrefix(QStringView text) const override
    {
        return text.startsWith(u'@');
    }
    QAbstractItemModel *model() const override
    {
        return m_model;
    }
    QString textWithoutPrefix(const QString &text) const override
    {
        return text.mid(1);
    }
    void setModel(UserListModel *model)
    {
        m_model = model;
    }

private:
    UserListModel *m_model = nullptr;
};

CompletionModel::CompletionModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_userCompletionProvider(new UserCompletionProvider(this))
{
    m_providers.append(m_userCompletionProvider);
}

ChatTextItemHelper *CompletionModel::textItem() const
{
    return m_textItem;
}

void CompletionModel::setTextItem(ChatTextItemHelper *textItem)
{
    if (textItem == m_textItem) {
        return;
    }

    if (m_textItem) {
        m_textItem->disconnect(this);
    }

    m_textItem = textItem;

    if (m_textItem) {
        connect(m_textItem, &ChatTextItemHelper::cursorPositionChanged, this, &CompletionModel::updateTextStart);
        connect(m_textItem, &ChatTextItemHelper::contentsChanged, this, &CompletionModel::updateCompletion);
    }
    Q_EMIT textItemChanged();
}

bool CompletionModel::isCompleting() const
{
    if (!m_textItem) {
        return false;
    }
    return m_textItem->isCompleting;
}

void CompletionModel::ignoreCurrentCompletion()
{
    m_ignoreCurrentCompletion = true;
    m_textItem->isCompleting = false;
    Q_EMIT isCompletingChanged();
}

void CompletionModel::updateTextStart()
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
    m_textStart = cursor.position() == 0 && cursor.selectedText() != u' ' ? 0 : cursor.position() + 1;
    updateCompletion();
}

QHash<int, QByteArray> CompletionModel::roleNames() const
{
    return {
        {DisplayNameRole, "displayName"},
        {SubtitleRole, "subtitle"},
        {IconNameRole, "iconName"},
        {ReplacedTextRole, "replacedText"},
        {HRefRole, "hRef"},
    };
}

void CompletionModel::updateCompletion()
{
    auto cursor = m_textItem->textCursor();
    if (cursor.isNull()) {
        return;
    }

    if (m_ignoreCurrentCompletion) {
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        if (cursor.selectedText() == u' ') {
            m_ignoreCurrentCompletion = false;
        }
        return;
    }

    cursor.setPosition(m_textStart);
    while (!cursor.selectedText().endsWith(u' ') && !cursor.atBlockEnd()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    const auto text = cursor.selectedText().trimmed();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    if (const auto &provider = providerForText(text)) {
        setSourceModel((*provider)->model());
        setFilterFixedString((*provider)->textWithoutPrefix(text));
    } else {
        setSourceModel({});
        setFilterFixedString({});
    }

    m_textItem->isCompleting = rowCount() > 0;
    Q_EMIT isCompletingChanged();
}

UserListModel *CompletionModel::userListModel() const
{
    return dynamic_cast<UserListModel *>(dynamic_cast<UserCompletionProvider *>(m_userCompletionProvider)->model());
}

void CompletionModel::setUserListModel(UserListModel *model)
{
    if (model == userListModel()) {
        return;
    }

    dynamic_cast<UserCompletionProvider *>(m_userCompletionProvider)->setModel(model);
    Q_EMIT userListModelChanged();
}

std::optional<CompletionProvider *> CompletionModel::providerForText(QStringView text) const
{
    for (const auto &provider : m_providers) {
        if (provider->matchesPrefix(text)) {
            return provider;
        }
    }
    return std::nullopt;
}

void CompletionModel::insertCompletion(const QString &text, const QUrl &link)
{
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
}

#include "moc_completionmodel.cpp"
