// SPDX-FileCopyrightText: None
// SPDX-License-Identifier: LGPL-2.0-or-later

#pragma once

#include <QVariant>

class EmojiTones
{
private:
    static QMultiHash<QString, QVariant> _tones;

    friend class EmojiModel;
};
