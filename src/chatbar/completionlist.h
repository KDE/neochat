// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QTextDocumentFragment>
#include <qqmlintegration.h>

class CompletionList : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")

public:
    explicit CompletionList(QObject *parent = nullptr);
    virtual ~CompletionList();

    /**
     * @brief Return the number of Completions in the list.
     */
    virtual qsizetype size() const = 0;

    /**
     * @brief Return the Completion at the given index.
     *
     * std::nullopt if i is not a valid index.
     *
     * @sa Completion
     */
    virtual QVariant data(qsizetype row, int role = Qt::DisplayRole) const = 0;

    /**
     * @brief Return the start sequence of Completions in the list.
     */
    virtual QString startSequence() const = 0;

Q_SIGNALS:
    void completionsAdded(CompletionList *list, qsizetype first, qsizetype last);

    void completionsRemoved(CompletionList *list, qsizetype first, qsizetype last);
};
