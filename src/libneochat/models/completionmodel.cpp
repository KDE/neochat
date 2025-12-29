// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "completionmodel.h"

#include <QDebug>
#include <QTextCursor>

#include "completionproxymodel.h"
#include "models/actionsmodel.h"
#include "models/customemojimodel.h"
#include "models/emojimodel.h"
#include "qmltextitemwrapper.h"
#include "userlistmodel.h"

CompletionModel::CompletionModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_textItem(new QmlTextItemWrapper(this))
    , m_filterModel(new CompletionProxyModel(this))
    , m_emojiModel(new QConcatenateTablesProxyModel(this))
{
    connect(m_textItem, &QmlTextItemWrapper::textItemChanged, this, &CompletionModel::textItemChanged);
    connect(m_textItem, &QmlTextItemWrapper::cursorPositionChanged, this, &CompletionModel::updateTextStart);
    connect(m_textItem, &QmlTextItemWrapper::contentsChanged, this, &CompletionModel::updateCompletion);

    m_emojiModel->addSourceModel(&CustomEmojiModel::instance());
    m_emojiModel->addSourceModel(&EmojiModel::instance());
}

QQuickItem *CompletionModel::textItem() const
{
    return m_textItem->textItem();
}

void CompletionModel::setTextItem(QQuickItem *textItem)
{
    m_textItem->setTextItem(textItem);
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

int CompletionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_autoCompletionType == None) {
        return 0;
    }
    return m_filterModel->rowCount();
}

QVariant CompletionModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_filterModel->rowCount()) {
        return {};
    }
    auto filterIndex = m_filterModel->index(index.row(), 0);
    if (m_autoCompletionType == User) {
        if (role == DisplayNameRole) {
            return m_filterModel->data(filterIndex, UserListModel::DisplayNameRole);
        }
        if (role == SubtitleRole) {
            return m_filterModel->data(filterIndex, UserListModel::UserIdRole);
        }
        if (role == IconNameRole) {
            return m_filterModel->data(filterIndex, UserListModel::AvatarRole);
        }
        if (role == ReplacedTextRole) {
            return m_filterModel->data(filterIndex, UserListModel::DisplayNameRole);
        }
        if (role == HRefRole) {
            return u"https://matrix.to/#/%1"_s.arg(m_filterModel->data(filterIndex, UserListModel::UserIdRole).toString());
        }
    }

    if (m_autoCompletionType == Command) {
        if (role == DisplayNameRole) {
            return u"%1 %2"_s.arg(m_filterModel->data(filterIndex, ActionsModel::Prefix).toString(),
                                  m_filterModel->data(filterIndex, ActionsModel::Parameters).toString());
        }
        if (role == SubtitleRole) {
            return m_filterModel->data(filterIndex, ActionsModel::Description);
        }
        if (role == IconNameRole) {
            return u"invalid"_s;
        }
        if (role == ReplacedTextRole) {
            return m_filterModel->data(filterIndex, ActionsModel::Prefix);
        }
    }
    if (m_autoCompletionType == Room) {
        if (role == DisplayNameRole) {
            return m_filterModel->data(filterIndex, RoomListModel::DisplayNameRole);
        }
        if (role == SubtitleRole) {
            return m_filterModel->data(filterIndex, RoomListModel::CanonicalAliasRole);
        }
        if (role == IconNameRole) {
            return m_filterModel->data(filterIndex, RoomListModel::AvatarRole).toString();
        }
        if (role == ReplacedTextRole) {
            return m_filterModel->data(filterIndex, RoomListModel::CanonicalAliasRole);
        }
        if (role == HRefRole) {
            return u"https://matrix.to/#/%1"_s.arg(m_filterModel->data(filterIndex, RoomListModel::CanonicalAliasRole).toString());
        }
    }
    if (m_autoCompletionType == Emoji) {
        if (role == DisplayNameRole) {
            return m_filterModel->data(filterIndex, CustomEmojiModel::DisplayRole);
        }
        if (role == IconNameRole) {
            return m_filterModel->data(filterIndex, CustomEmojiModel::MxcUrl);
        }
        if (role == ReplacedTextRole) {
            return m_filterModel->data(filterIndex, CustomEmojiModel::ReplacedTextRole);
        }
        if (role == SubtitleRole) {
            return m_filterModel->data(filterIndex, EmojiModel::DescriptionRole);
        }
    }

    return {};
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
    cursor.setPosition(m_textStart);
    while (!cursor.selectedText().endsWith(u' ') && !cursor.atBlockEnd()) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    }
    const auto text = cursor.selectedText().trimmed();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    const auto fullText = cursor.selectedText();

    if (text.startsWith(QLatin1Char('@'))) {
        m_filterModel->setSourceModel(m_userListModel);
        m_filterModel->setFilterRole(UserListModel::UserIdRole);
        m_filterModel->setSecondaryFilterRole(UserListModel::DisplayNameRole);
        m_filterModel->setFullText(fullText);
        m_filterModel->setFilterText(text);
        m_autoCompletionType = User;
        m_filterModel->invalidate();
    } else if (text.startsWith(QLatin1Char('/'))) {
        m_filterModel->setSourceModel(&ActionsModel::instance());
        m_filterModel->setFilterRole(ActionsModel::Prefix);
        m_filterModel->setSecondaryFilterRole(-1);
        m_filterModel->setFullText(fullText);
        m_filterModel->setFilterText(text.mid(1));
        m_autoCompletionType = Command;
        m_filterModel->invalidate();
    } else if (text.startsWith(QLatin1Char('#'))) {
        m_autoCompletionType = Room;
        m_filterModel->setSourceModel(m_roomListModel);
        m_filterModel->setFilterRole(RoomListModel::CanonicalAliasRole);
        m_filterModel->setSecondaryFilterRole(RoomListModel::DisplayNameRole);
        m_filterModel->setFullText(fullText);
        m_filterModel->setFilterText(text);
        m_filterModel->invalidate();
    } else if (text.startsWith(QLatin1Char(':')) && text.size() > 1 && !text[1].isUpper()
               && (fullText.indexOf(QLatin1Char(':'), 1) == -1
                   || (fullText.indexOf(QLatin1Char(' ')) != -1 && fullText.indexOf(QLatin1Char(':'), 1) > fullText.indexOf(QLatin1Char(' '), 1)))) {
        m_filterModel->setSourceModel(m_emojiModel);
        m_autoCompletionType = Emoji;
        m_filterModel->setFilterRole(CustomEmojiModel::Name);
        m_filterModel->setSecondaryFilterRole(EmojiModel::DescriptionRole);
        m_filterModel->setFullText(fullText);
        m_filterModel->setFilterText(text);
        m_filterModel->invalidate();
    } else {
        m_autoCompletionType = None;
    }
    beginResetModel();
    endResetModel();
}

CompletionModel::AutoCompletionType CompletionModel::autoCompletionType() const
{
    return m_autoCompletionType;
}

void CompletionModel::setAutoCompletionType(AutoCompletionType autoCompletionType)
{
    m_autoCompletionType = autoCompletionType;
    Q_EMIT autoCompletionTypeChanged();
}

RoomListModel *CompletionModel::roomListModel() const
{
    return m_roomListModel;
}

void CompletionModel::setRoomListModel(RoomListModel *roomListModel)
{
    m_roomListModel = roomListModel;
    Q_EMIT roomListModelChanged();
}

UserListModel *CompletionModel::userListModel() const
{
    return m_userListModel;
}

void CompletionModel::setUserListModel(UserListModel *userListModel)
{
    if (userListModel == m_userListModel) {
        return;
    }

    m_userListModel = userListModel;
    Q_EMIT userListModelChanged();
}

#include "moc_completionmodel.cpp"
