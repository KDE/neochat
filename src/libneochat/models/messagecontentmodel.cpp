// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "messagecontentmodel.h"

#include <QStyleHints>

#include <KLocalizedString>

#include "block.h"
#include "blocklogging.h"

#include "texthandler.h"

using namespace Quotient;

MessageContentModel::MessageContentModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &MessageContentModel::updateSpoilers);
}

QString MessageContentModel::threadRootId() const
{
    return {};
}

QVariant MessageContentModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    if (index.row() < 0 || index.row() >= rowCount()) {
        qCWarning(BlocksLog) << __FUNCTION__ << "called with invalid index" << index << rowCount();
        return {};
    }

    const auto &component = m_components[index.row()];
    if (!component) {
        return {};
    }

    if (role == ComponentTypeRole) {
        return component->type();
    }
    if (role == BlockRole) {
        return component->toVariant();
    }
    if (role == ReplyContentModelRole) {
        return QVariant::fromValue<MessageContentModel *>(m_replyModel);
    }
    if (role == ThreadRootRole) {
        return threadRootId();
    }
    if (role == EditableRole) {
        return m_editableActive;
    }
    if (role == CurrentFocusRole) {
        return index.row() == m_currentFocusComponent.row();
    }

    return {};
}

int MessageContentModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_components.size();
}

QHash<int, QByteArray> MessageContentModel::roleNames() const
{
    return roleNamesStatic();
}

QHash<int, QByteArray> MessageContentModel::roleNamesStatic()
{
    QHash<int, QByteArray> roles;
    roles[MessageContentModel::ComponentTypeRole] = "componentType";
    roles[MessageContentModel::BlockRole] = "block";
    roles[MessageContentModel::ReplyContentModelRole] = "replyContentModel";
    roles[MessageContentModel::ThreadRootRole] = "threadRoot";
    roles[MessageContentModel::EditableRole] = "editable";
    roles[MessageContentModel::CurrentFocusRole] = "currentFocus";
    return roles;
}

bool MessageContentModel::hasComponentType(Blocks::Type type) const
{
    return std::find_if(m_components.cbegin(),
                        m_components.cend(),
                        [type](Blocks::Block *component) {
                            return component->type() == type;
                        })
        != m_components.cend();
}

bool MessageContentModel::hasComponentType(const QList<Blocks::Type> &types) const
{
    return std::ranges::any_of(types, [this](const Blocks::Type &type) {
        return hasComponentType(type);
    });
}

void MessageContentModel::forEachComponentOfType(Blocks::Type type, std::function<Blocks::BlockPtrsIt(Blocks::BlockPtrsIt)> function)
{
    auto it = m_components.begin();
    while ((it = std::find_if(it,
                              m_components.end(),
                              [type](const Blocks::Block *component) {
                                  return component->type() == type;
                              }))
           != m_components.end()) {
        it = function(it);
    }
}

void MessageContentModel::forEachComponentOfType(QList<Blocks::Type> types, std::function<Blocks::BlockPtrsIt(Blocks::BlockPtrsIt)> function)
{
    for (const auto &type : types) {
        forEachComponentOfType(type, function);
    }
}

void MessageContentModel::updateSpoilers()
{
    for (auto it = m_components.begin(); it != m_components.end(); ++it) {
        updateSpoiler(index(it - m_components.begin()));
    }
}

void MessageContentModel::updateSpoiler(const QModelIndex &index)
{
    const auto row = index.row();
    if (row < 0 || row >= rowCount(index.parent())) {
        qCWarning(BlocksLog) << __FUNCTION__ << "called with invalid index" << index << rowCount();
        return;
    }

    const auto textBlock = dynamic_cast<Blocks::TextBlock *>(m_components[row]);
    if (!textBlock) {
        return;
    }
    const auto item = textBlock->item();
    if (!item) {
        return;
    }
    const auto doc = item->document();
    if (!doc) {
        return;
    }

    const auto newText = TextHandler::updateSpoilerText(this, doc->toHtml(), textBlock->spoilerRevealed());
    doc->clear();
    doc->setHtml(newText);
    Q_EMIT dataChanged(index, index, {BlockRole});
}

void MessageContentModel::toggleSpoiler(QModelIndex index)
{
    const auto row = index.row();
    if (row < 0 || row >= rowCount()) {
        qCWarning(BlocksLog) << __FUNCTION__ << "called with invalid row" << row << m_components.size();
        return;
    }
    const auto textBlock = dynamic_cast<Blocks::TextBlock *>(m_components[row]);
    if (!textBlock) {
        return;
    }

    textBlock->setSpoilerRevealed(!textBlock->spoilerRevealed());
    Q_EMIT dataChanged(index, index, {BlockRole});
    updateSpoiler(index);
}

#include "moc_messagecontentmodel.cpp"
