// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtLocation 5.15
import QtPositioning 5.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

TimelineContainer {
    id: locationDelegate

    ColumnLayout {
        Layout.maximumWidth: locationDelegate.contentMaxWidth
        Layout.preferredWidth: locationDelegate.contentMaxWidth
        Map {
            id: map
            Layout.fillWidth: true
            Layout.preferredHeight: locationDelegate.contentMaxWidth / 16 * 9

            center: QtPositioning.coordinate(model.latitude, model.longitude)
            zoomLevel: 15
            plugin: Plugin {
                name: "osm"
                PluginParameter {
                    name: "osm.useragent"
                    value: Application.name + "/" + Application.version + " (kde-devel@kde.org)"
                }
                PluginParameter {
                    name: "osm.mapping.providersrepository.address"
                    value: "https://autoconfig.kde.org/qtlocation/"
                }
            }

            onCopyrightLinkActivated: Qt.openUrlExternally(link)


            MapQuickItem {
                id: point

                anchorPoint.x: sourceItem.width / 2
                anchorPoint.y: sourceItem.height / 2
                coordinate: QtPositioning.coordinate(model.latitude, model.longitude)
                autoFadeIn: false

                sourceItem: Kirigami.Icon {
                    width: height
                    height: Kirigami.Units.iconSizes.medium
                    source: "flag-blue"
                }
            }

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onLongPressed: openMessageContext(model, "", model.message)
            }
            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: openMessageContext(model, "", model.message)
            }
        }
    }
}
