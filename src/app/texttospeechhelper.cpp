// SPDX-FileCopyrightText: 2025 Ritchie Frodomar <ritchie@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "texttospeechhelper.h"

void TextToSpeechHelper::speak(const QString &textToSpeak)
{
    if (!m_speech) {
        m_speech = new QTextToSpeech();
    }

    m_speech->say(textToSpeak);
}

#include "moc_texttospeechhelper.cpp"