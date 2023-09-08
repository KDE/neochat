// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import QtQuick 2.7
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.14 as Kirigami

/**
 * Action that allows an user to share data with other apps and service
 * installed on their computer. The goal of this high level API is to
 * adapte itself for each platform and adopt the native component.
 *
 * TODO add Android support
 */
Kirigami.Action {
    id: shareAction
    icon.name: "emblem-shared-symbolic"
    text: i18n("Share")
    tooltip: i18n("Share the selected media")

    property var doBeforeSharing: () => {}
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
    property var inputData: ({})

    property Instantiator _instantiator: Instantiator {
        Component.onCompleted: {
            const purposeModel = Qt.createQmlObject('import org.kde.purpose 1.0 as Purpose;
Purpose.PurposeAlternativesModel {
    pluginType: "Export"
}', shareAction._instantiator);
            purposeModel.inputData = Qt.binding(function() {
                return shareAction.inputData;
            });
            _instantiator.model = purposeModel;
            shareAction.visible = true;
        }

        delegate: Kirigami.Action {
            property int index
            text: model.display
            icon.name: model.iconName
            onTriggered: {
                doBeforeSharing();
                applicationWindow().pageStack.pushDialogLayer('qrc:/ShareDialog.qml', {
                    title: shareAction.tooltip,
                    index: index,
                    model: shareAction._instantiator.model
                })
            }
        }
        onObjectAdded: (index, object) => {
            object.index = index;
            shareAction.children.push(object)
        }
        onObjectRemoved: (index, object) => shareAction.children = Array.from(shareAction.children).filter(obj => obj.pluginId !== object.pluginId)
    }
}
