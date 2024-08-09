# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(login STATIC)
ecm_add_qml_module(login GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.login
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/login
    QML_FILES
        WelcomePage.qml
        LoginStep.qml
        Captcha.qml
        Email.qml
        Homeserver.qml
        Loading.qml
        Login.qml
        LoginMethod.qml
        LoginRegister.qml
        Password.qml
        RegisterPassword.qml
        Sso.qml
        Terms.qml
        Username.qml
        Oidc.qml
)
