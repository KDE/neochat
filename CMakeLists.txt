# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <fella@posteo.de>
# SPDX-FileCopyrightText: 2021 Adriaan de Groot <groot@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

project(NeoChat)

set(KF5_MIN_VERSION "5.86.0")
set(QT_MIN_VERSION "5.15.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KDE_COMPILERSETTINGS_LEVEL 5.84)

include(FeatureSummary)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(ECMQMLModules)
include(KDEClangFormat)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(KDEGitCommitHooks)
include(ECMCheckOutboundLicense)

if(NEOCHAT_FLATPAK)
    include(cmake/Flatpak.cmake)
endif()

# Fix a crash due to problems with quotient's event system. Can probably be removed once the reworked event system is in
cmake_policy(SET CMP0063 OLD)

ecm_setup_version(1.2.80
    VARIABLE_PREFIX NEOCHAT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/neochat-version.h
)

find_package(Qt5 ${QT_MIN_VERSION} NO_MODULE COMPONENTS Core Quick Gui QuickControls2 Multimedia Svg)
set_package_properties(Qt5 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
find_package(KF5 ${KF5_MIN_VERSION} COMPONENTS Kirigami2 I18n Notifications Config CoreAddons)
set_package_properties(KF5 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
set_package_properties(KF5Kirigami2 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Kirigami application UI framework"
)

find_package(Qt5Keychain)
set_package_properties(Qt5Keychain PROPERTIES
    TYPE REQUIRED
    PURPOSE "Secure storage of account secrets"
)

if(ANDROID)
    find_package(OpenSSL)
    set_package_properties(OpenSSL PROPERTIES
        TYPE REQUIRED
        PURPOSE "Encrypted communications"
    )
else()
    find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS Widgets)
    find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS QQC2DesktopStyle ConfigWidgets KIO WindowSystem Sonnet)
    set_package_properties(KF5QQC2DesktopStyle PROPERTIES
        TYPE RUNTIME
    )
    ecm_find_qmlmodule(org.kde.syntaxhighlighting 1.0)
endif()

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    find_package(KF5DBusAddons ${KF5_MIN_VERSION} REQUIRED)
endif()

find_package(Quotient 0.6)
set_package_properties(Quotient PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Qt wrapper arround Matrix API"
    URL "https://github.com/quotient-im/libQuotient/"
    PURPOSE "Talk with matrix server"
)

find_package(cmark)
set_package_properties(cmark PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Cmark is the common mark reference implementation"
    URL "https://github.com/commonmark/cmark"
    PURPOSE "Convert markdown to html"
)

ecm_find_qmlmodule(org.kde.kquickimageeditor 1.0)
ecm_find_qmlmodule(org.kde.kitemmodels 1.0)

find_package(KQuickImageEditor COMPONENTS)
set_package_properties(KQuickImageEditor PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Simple image editor for QtQuick applications"
    URL "https://invent.kde.org/libraries/kquickimageeditor/"
    PURPOSE "Add image editing capability to image attachments"
)

find_package(QCoro REQUIRED)

qcoro_enable_coroutines()

if(ANDROID)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/android/version.gradle.in ${CMAKE_BINARY_DIR}/version.gradle)
endif()

install(FILES org.kde.neochat.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.neochat.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.neochat.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES org.kde.neochat-symbolic.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/16x16/apps RENAME org.kde.neochat.svg)
install(FILES org.kde.neochat-symbolic.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/16x16@2/apps RENAME org.kde.neochat.svg)
install(FILES org.kde.neochat-symbolic.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/16x16@3/apps RENAME org.kde.neochat.svg)
install(FILES org.kde.neochat-symbolic.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/symbolic/apps)
install(FILES neochat.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})

add_definitions(-DQT_NO_FOREACH)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h *.qml)
ecm_check_outbound_license(LICENSES GPL-3.0-only FILES ${ALL_SOURCE_FILES})
