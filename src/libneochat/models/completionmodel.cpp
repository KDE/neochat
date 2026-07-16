// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "completionmodel.h"

#include <QTextCursor>

#include <Kirigami/Platform/PlatformTheme>

#include "chattextitemhelper.h"
#include "completionproxymodel.h"
#include "models/actionsmodel.h"
#include "models/customemojimodel.h"
#include "models/emojimodel.h"
#include "models/roomlistmodel.h"
#include "userfiltermodel.h"
#include "userlistmodel.h"

CompletionModel::CompletionModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_textItem(new ChatTextItemHelper(this))
    , m_emojiModel(new QConcatenateTablesProxyModel(this))
{
    m_emojiModel->addSourceModel(&CustomEmojiModel::instance());
    m_emojiModel->addSourceModel(&EmojiModel::instance());

    m_userFilterModel = new CompletionProxyModel(this);
    m_userFilterModel->setSourceModel(m_userListModel);
    m_userFilterModel->setFilterRole(UserListModel::UserIdRole);
    m_userFilterModel->setSecondaryFilterRole(UserListModel::DisplayNameRole);

    m_commandFilterModel = new CompletionProxyModel(this);
    m_commandFilterModel->setSourceModel(&ActionsModel::instance());
    m_commandFilterModel->setFilterRole(ActionsModel::Prefix);
    m_commandFilterModel->setSecondaryFilterRole(-1);

    m_roomFilterModel = new CompletionProxyModel(this);
    m_roomFilterModel->setSourceModel(m_roomListModel);
    m_roomFilterModel->setFilterRole(RoomListModel::CanonicalAliasRole);
    m_roomFilterModel->setSecondaryFilterRole(RoomListModel::DisplayNameRole);

    m_emojiFilterModel = new CompletionProxyModel(this);
    m_emojiFilterModel->setSourceModel(m_emojiModel);
    m_emojiFilterModel->setFilterRole(CustomEmojiModel::Name);
    m_emojiFilterModel->setSecondaryFilterRole(EmojiModel::DescriptionRole);
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

    if (m_textItem->isCompleting) {
        m_textItem->isCompleting = false;
        Q_EMIT isCompletingChanged();
    }
}

CompletionProxyModel *CompletionModel::modelForCurrentType() const
{
    switch (m_autoCompletionType) {
    case User:
        return m_userFilterModel;
    case Room:
        return m_roomFilterModel;
    case Emoji:
        return m_emojiFilterModel;
    case Command:
        return m_commandFilterModel;
    default:
        break;
    }

    return nullptr;
}

void CompletionModel::connectModelSignals(CompletionProxyModel *model)
{
    connect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &CompletionModel::rowsAboutToBeInserted);
    connect(model, &QAbstractItemModel::rowsInserted, this, &CompletionModel::rowsInserted);
    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &CompletionModel::rowsAboutToBeRemoved);
    connect(model, &QAbstractItemModel::rowsRemoved, this, &CompletionModel::rowsRemoved);
    connect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, &CompletionModel::rowsAboutToBeMoved);
    connect(model, &QAbstractItemModel::rowsMoved, this, &CompletionModel::rowsMoved);
    connect(model, &QAbstractItemModel::dataChanged, this, &CompletionModel::dataChanged);
}

void CompletionModel::disconnectModelSignals(CompletionProxyModel *model)
{
    disconnect(model, &QAbstractItemModel::rowsAboutToBeInserted, this, &CompletionModel::rowsAboutToBeInserted);
    disconnect(model, &QAbstractItemModel::rowsInserted, this, &CompletionModel::rowsInserted);
    disconnect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &CompletionModel::rowsAboutToBeRemoved);
    disconnect(model, &QAbstractItemModel::rowsRemoved, this, &CompletionModel::rowsRemoved);
    disconnect(model, &QAbstractItemModel::rowsAboutToBeMoved, this, &CompletionModel::rowsAboutToBeMoved);
    disconnect(model, &QAbstractItemModel::rowsMoved, this, &CompletionModel::rowsMoved);
    disconnect(model, &QAbstractItemModel::dataChanged, this, &CompletionModel::dataChanged);
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
    Q_UNUSED(parent)

    const auto model = modelForCurrentType();
    if (!model) {
        return 0;
    }
    return model->rowCount();
}

QVariant CompletionModel::data(const QModelIndex &index, int role) const
{
    const auto model = modelForCurrentType();
    if (!model || index.row() < 0 || index.row() >= model->rowCount()) {
        return {};
    }
    auto filterIndex = model->index(index.row(), 0);
    if (m_autoCompletionType == User) {
        if (role == DisplayNameRole) {
            return m_userFilterModel->data(filterIndex, UserListModel::DisplayNameRole);
        }
        if (role == SubtitleRole) {
            return m_userFilterModel->data(filterIndex, UserListModel::UserIdRole);
        }
        if (role == IconNameRole) {
            return m_userFilterModel->data(filterIndex, UserListModel::AvatarRole);
        }
        if (role == ReplacedTextRole) {
            return m_userFilterModel->data(filterIndex, UserListModel::DisplayNameRole);
        }
        if (role == HRefRole) {
            return u"https://matrix.to/#/%1"_s.arg(m_userFilterModel->data(filterIndex, UserListModel::UserIdRole).toString());
        }
    }

    if (m_autoCompletionType == Command) {
        if (role == DisplayNameRole) {
            return u"%1 %2"_s.arg(m_commandFilterModel->data(filterIndex, ActionsModel::Prefix).toString(),
                                  m_commandFilterModel->data(filterIndex, ActionsModel::Parameters).toString());
        }
        if (role == SubtitleRole) {
            return m_commandFilterModel->data(filterIndex, ActionsModel::Description);
        }
        if (role == IconNameRole) {
            return u"invalid"_s;
        }
        if (role == ReplacedTextRole) {
            return m_commandFilterModel->data(filterIndex, ActionsModel::Prefix);
        }
    }
    if (m_autoCompletionType == Room) {
        if (role == DisplayNameRole) {
            return m_roomFilterModel->data(filterIndex, RoomListModel::DisplayNameRole);
        }
        if (role == SubtitleRole) {
            return m_roomFilterModel->data(filterIndex, RoomListModel::CanonicalAliasRole);
        }
        if (role == IconNameRole) {
            return m_roomFilterModel->data(filterIndex, RoomListModel::AvatarRole).toString();
        }
        if (role == ReplacedTextRole) {
            return m_roomFilterModel->data(filterIndex, RoomListModel::CanonicalAliasRole);
        }
        if (role == HRefRole) {
            return u"https://matrix.to/#/%1"_s.arg(m_roomFilterModel->data(filterIndex, RoomListModel::CanonicalAliasRole).toString());
        }
    }
    if (m_autoCompletionType == Emoji) {
        if (role == DisplayNameRole) {
            return m_emojiFilterModel->data(filterIndex, CustomEmojiModel::DisplayRole);
        }
        if (role == IconNameRole) {
            return m_emojiFilterModel->data(filterIndex, CustomEmojiModel::MxcUrl);
        }
        if (role == ReplacedTextRole) {
            return m_emojiFilterModel->data(filterIndex, CustomEmojiModel::ReplacedTextRole);
        }
        if (role == SubtitleRole) {
            return m_emojiFilterModel->data(filterIndex, EmojiModel::DescriptionRole);
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
    const auto fullText = cursor.selectedText();

    const auto setNewAutoCompletion = [this](AutoCompletionType type) {
        if (m_autoCompletionType != type) {
            if (const auto oldModel = modelForCurrentType()) {
                disconnectModelSignals(oldModel);
            }

            m_autoCompletionType = type;

            // The underlying model changed, so we have no choice but to wipe everything.
            beginResetModel();
            endResetModel();

            if (const auto newModel = modelForCurrentType()) {
                connectModelSignals(newModel);
            }
        }

        const bool isCompleting = rowCount() > 0;
        if (m_textItem->isCompleting != isCompleting) {
            m_textItem->isCompleting = isCompleting;
            Q_EMIT isCompletingChanged();
        }
    };

    // Don't start filtering anything until they type one more character, because this can load a lot of users, rooms or emojis.
    if (text.size() > 1) {
        if (text.startsWith(QLatin1Char('@'))) {
            // Users
            m_userFilterModel->setFilterText(text);

            setNewAutoCompletion(User);
            return;
        }
        if (text.startsWith(QLatin1Char('/'))) {
            // Commands
            m_commandFilterModel->setFilterText(text.mid(1));

            setNewAutoCompletion(Command);
            return;
        }
        if (text.startsWith(QLatin1Char('#'))) {
            // Rooms
            m_roomFilterModel->setFilterText(text);

            setNewAutoCompletion(Room);
            return;
        }
        if (text.startsWith(QLatin1Char(':')) && !text[1].isUpper()) {
            // Emojis
            const qsizetype locationOfEndColon = fullText.indexOf(QLatin1Char(':'), 1);
            const qsizetype locationOfSpace = fullText.indexOf(QLatin1Char(' '));

            if (locationOfEndColon == -1 || locationOfEndColon + 1 == fullText.size() || (locationOfSpace != -1 && locationOfEndColon > locationOfSpace)) {
                m_emojiFilterModel->setFilterText(text);

                setNewAutoCompletion(Emoji);
                return;
            }
        }
    }

    setNewAutoCompletion(None);
}

RoomListModel *CompletionModel::roomListModel() const
{
    return m_roomListModel;
}

void CompletionModel::setRoomListModel(RoomListModel *roomListModel)
{
    if (roomListModel == m_roomListModel) {
        return;
    }

    m_roomListModel = roomListModel;
    m_roomFilterModel->setSourceModel(roomListModel);
    Q_EMIT roomListModelChanged();
}

UserFilterModel *CompletionModel::userListModel() const
{
    return m_userListModel;
}

void CompletionModel::setUserFilterModel(UserFilterModel *userListModel)
{
    if (userListModel == m_userListModel) {
        return;
    }

    m_userListModel = userListModel;
    m_userFilterModel->setSourceModel(m_userListModel);
    Q_EMIT userListModelChanged();
}

CompletionModel::AutoCompletionType CompletionModel::autoCompletionType() const
{
    return m_autoCompletionType;
}

void CompletionModel::setAutoCompletionType(AutoCompletionType autoCompletionType)
{
    m_autoCompletionType = autoCompletionType;
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
