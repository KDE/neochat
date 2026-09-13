// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QTextDocumentFragment>
#include <qqmlintegration.h>

struct Completion {
    QString title;
    QString description;
    QUrl avatarSource;
    QString startsequence;
    QStringList matchSequences;
    QString replaceString;
    QUrl hRef;
};

class CompletionList : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit CompletionList(QObject *parent = nullptr);
    virtual ~CompletionList();

    /**
     * @brief Return the number of Completions in the list.
     */
    virtual qsizetype size() const;

    /**
     * @brief Return the Completion at the given index.
     *
     * std::nullopt if i is not a valid index.
     *
     * @sa Completion
     */
    virtual std::optional<Completion> at(qsizetype i) const;

Q_SIGNALS:
    void completionsAdded(CompletionList *list, qsizetype first, qsizetype last);

    void completionsRemoved(CompletionList *list, qsizetype first, qsizetype last);
};
