// SPDX-FileCopyrightText: 2025 Ritchie Frodomar <ritchie@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QtTextToSpeech>

class TextToSpeechHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    Q_INVOKABLE void speak(const QString &textToSpeak);

private:
    QTextToSpeech *m_speech = nullptr;
};