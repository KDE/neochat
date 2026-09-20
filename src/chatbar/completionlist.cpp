// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "completionlist.h"

CompletionList::CompletionList(QObject *parent)
    : QObject(parent)
{
}

CompletionList::~CompletionList()
{
}

#include "moc_completionlist.cpp"
