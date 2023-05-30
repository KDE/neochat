// SPDX-FileCopyrightText: Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QObject>

class QQuickTextDocument;
class QTextDocument;

class EmojiFixer : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void addTextDocument(QQuickTextDocument *doc);

private:
    void fix(QTextDocument *doc);
};
