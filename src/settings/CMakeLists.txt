# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(settings STATIC)

set_source_files_properties(
    RoomSettingsView.qml
    NeoChatSettingsView.qml
    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

ecm_add_qml_module(settings GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.settings
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/settings
    QML_FILES
        NeoChatSettingsView.qml
        RoomSettingsView.qml
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
        ColorScheme.qml
        DevicesCard.qml
        DeviceDelegate.qml
        EmoticonFormCard.qml
        IdentityServerDelegate.qml
        IgnoredUsersDialog.qml
        NotificationRuleItem.qml
        PasswordSheet.qml
        ThemeRadioButton.qml
        ThreePIdCard.qml
)
