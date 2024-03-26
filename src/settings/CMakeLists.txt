# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(settings STATIC)
qt_add_qml_module(settings
    URI org.kde.neochat.settings
    QML_FILES
        NeoChatSettings.qml
        RoomSettings.qml
        About.qml
        AboutKDE.qml
        AccountsPage.qml
        AccountEditorPage.qml
        AppearanceSettingsPage.qml
        DevicesPage.qml
        EmoticonsPage.qml
        EmoticonEditorPage.qml
        GlobalNotificationsPage.qml
        NeoChatGeneralPage.qml
        NeoChatSecurityPage.qml
        NetworkProxyPage.qml
        Permissions.qml
        PushNotification.qml
        RoomGeneralPage.qml
        RoomSecurityPage.qml
        SonnetConfigPage.qml
        ColorScheme.qml
        DevicesCard.qml
        DeviceDelegate.qml
        EmoticonFormCard.qml
        IgnoredUsersDialog.qml
        NotificationRuleItem.qml
        ThemeRadioButton.qml
)
