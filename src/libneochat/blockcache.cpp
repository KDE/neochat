// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "blockcache.h"

#include "chattextitemhelper.h"

using namespace Block;

void Cache::fill(QList<MessageComponent> components)
{
    std::ranges::for_each(components, [this](const MessageComponent &component) {
        if (!MessageComponentType::isTextType(component.type)) {
            return;
        }
        const auto textItem = component.attributes["chatTextItemHelper"_L1].value<ChatTextItemHelper *>();
        if (!textItem) {
            return;
        }
        append(CacheItem{
            .type = component.type,
            .content = textItem->toFragment(),
        });
    });
}
