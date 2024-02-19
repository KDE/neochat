// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.purpose as Purpose

import org.kde.neochat

/**
 * Action that allows a user to share data with other apps and services
 * installed on their computer. The goal of this high level API is to
 * adapt itself for each platform and adopt the native component.
 *
 * TODO add Android support
 */
Kirigami.Action {
    id: root

    icon.name: "emblem-shared-symbolic"
    text: i18n("Share")
    tooltip: i18n("Share the selected media")

    visible: false

    /**
     * This property holds the input data for purpose.
     *
     * @code{.qml}
     * Purpose.ShareAction {
     *     inputData: {
     *         'urls': ['file://home/notroot/Pictures/mypicture.png'],
     *         'mimeType': ['image/png']
     *     }
     * }
     * @endcode
     */
    property var inputData

    required property string eventId
    required property NeoChatRoom room

    property Instantiator _instantiator: Instantiator {
        model: Purpose.PurposeAlternativesModel {
            pluginType: "Export"
            inputData: root.inputData
        }

        delegate: Kirigami.Action {
            property int index
            text: model.display
            icon.name: model.iconName
            onTriggered: {
                root.room.download(root.eventId, root.inputData.urls[0]);
                root.room.fileTransferCompleted.connect(share);
            }
            function share(id) {
                if (id != root.eventId) {
                    return;
                }
                applicationWindow().pageStack.pushDialogLayer('qrc:/org/kde/neochat/qml/ShareDialog.qml', {
                    title: root.text,
                    index: index,
                    model: root._instantiator.model
                }, {
                    title: i18nc("@title", "Share")
                });
                root.room.fileTransferCompleted.disconnect(share);
            }
        }
        onObjectAdded: (index, object) => {
            object.index = index;
            root.children.push(object);
        }
        onObjectRemoved: (index, object) => root.children = Array.from(root.children).filter(obj => obj.pluginId !== object.pluginId)
    }
}
