// SPDX-FileCopyrightText: 2025 Ritchie Frodomar <ritchie@kde.org>

#include "texttospeechhelper.h"

void TextToSpeechHelper::speak(const QString &textToSpeak)
{
    if (!m_speech) {
        m_speech = new QTextToSpeech();
    }

    m_speech->say(textToSpeak);
}

#include "moc_texttospeechhelper.cpp"