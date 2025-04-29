// SPDX-FileCopyrightText: 2025 Ritchie Frodomar <ritchie@kde.org>

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QtTextToSpeech>

class TextToSpeechHelper : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

private:
    QTextToSpeech *m_speech = nullptr;

public:
    Q_INVOKABLE void speak(const QString &textToSpeak);
};