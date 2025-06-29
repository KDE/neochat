// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import Qt.labs.qmlmodels

import org.kde.neochat

/**
 * @brief Select a message component based on a MessageComponentType.
 */
BaseMessageComponentChooser {
    id: root

    DelegateChoice {
        roleValue: MessageComponentType.ThreadBody
        delegate: ThreadBodyComponent {
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
            onHoveredLinkChanged: hoveredLink => {
                root.hoveredLinkChanged(hoveredLink);
            }
        }
    }
}
