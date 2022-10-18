// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QQuickItem>
#include <QQuickTextDocument>

class MessageFormatter : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE QString format(const QString &messageBody, QQuickTextDocument *doc, QQuickItem *item);
    Q_INVOKABLE QString formatInternal(const QString &messageBody, QTextDocument *doc);
};