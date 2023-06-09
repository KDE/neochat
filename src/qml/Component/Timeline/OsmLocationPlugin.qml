// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma Singleton

import QtQuick 2.15
import QtLocation 5.15

QtObject {
    property var plugin: Plugin {
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
}
