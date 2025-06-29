// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "pollanswermodel.h"

#include "neochatroom.h"
#include "pollhandler.h"

PollAnswerModel::PollAnswerModel(PollHandler *parent)
    : QAbstractListModel(parent)
{
    Q_ASSERT(parent != nullptr);

    connect(parent, &PollHandler::selectionsChanged, this, [this]() {
        dataChanged(index(0), index(rowCount() - 1), {CountRole, LocalChoiceRole, IsWinnerRole});
    });
    connect(parent, &PollHandler::answersChanged, this, [this]() {
        dataChanged(index(0), index(rowCount() - 1), {TextRole});
    });
}

QVariant PollAnswerModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid));

    const auto row = index.row();
    if (row < 0 || row >= rowCount()) {
        return {};
    }

    const auto pollHandler = dynamic_cast<PollHandler *>(this->parent());
    if (pollHandler == nullptr) {
        qWarning() << "PollAnswerModel created with nullptr parent.";
        return 0;
    }

    if (role == IdRole) {
        return pollHandler->answerAtRow(row).id;
    }
    if (role == TextRole) {
        return pollHandler->answerAtRow(row).text;
    }
    if (role == CountRole) {
        return pollHandler->answerCountAtId(pollHandler->answerAtRow(row).id);
    }
    if (role == LocalChoiceRole) {
        const auto room = pollHandler->room();
        if (room == nullptr) {
            return {};
        }
        return pollHandler->checkMemberSelectedId(room->localMember().id(), pollHandler->answerAtRow(row).id);
    }
    if (role == IsWinnerRole) {
        return pollHandler->winningAnswerIds().contains(pollHandler->answerAtRow(row).id) && pollHandler->hasEnded();
    }
    return {};
}

int PollAnswerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    const auto pollHandler = dynamic_cast<PollHandler *>(this->parent());
    if (pollHandler == nullptr) {
        qWarning() << "PollAnswerModel created with nullptr parent.";
        return 0;
    }

    return pollHandler->numAnswers();
}

QHash<int, QByteArray> PollAnswerModel::roleNames() const
{
    return {
        {IdRole, "id"},
        {TextRole, "answerText"},
        {CountRole, "count"},
        {LocalChoiceRole, "localChoice"},
        {IsWinnerRole, "isWinner"},
    };
}
