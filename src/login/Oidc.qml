// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

LoginStep {
    id: root
    FormCard.FormTextFieldDelegate {
        id: homeserver

        visible: !infoLabel.visible
        label: i18n("Homeserver")
        text: "synapse-oidc.element.dev"
    }

    Connections {
        target: Controller.pendingOidcConnection
        function onOpenOidcUrl(url: string): void {
            infoLabel.url = url;
            infoLabel.visible = true;
        }
        function onConnected(): void {
            root.processed("Loading");
        }
    }

    FormCard.FormTextDelegate {
        id: infoLabel

        property string url: ""
        visible: false
        text: i18n("To continue, please open the following link in your web browser: %1", "<br /><br /><a href='" + url + "'>" + url + "</a>")
        onLinkActivated: url => Qt.openUrlExternally(url)
    }
    FormCard.FormDelegateSeparator { above: openLink }

    FormCard.FormButtonDelegate {
        id: openLink
        visible: infoLabel.visible
        text: i18n("Open Authorization Link")
        icon.name: "document-open"
        onClicked: Qt.openUrlExternally(infoLabel.url.authorizeUrl)
    }

    FormCard.FormDelegateSeparator {
        visible: infoLabel.visible
        above: openLink
        below: copyLink
    }

    FormCard.FormButtonDelegate {
        id: copyLink

        visible: infoLabel.visible
        text: i18n("Copy Authorization Link")
        icon.name: "edit-copy"
        onClicked: {
            Clipboard.saveText(infoLabel.url)
            applicationWindow().showPassiveNotification(i18n("Link copied."));
        }
    }

    FormCard.FormButtonDelegate {
        visible: !infoLabel.visible
        text: i18nc("@action:button", "Log in with OIDC")
        onClicked: Controller.startOidcLogin(homeserver.text)

    }
}
