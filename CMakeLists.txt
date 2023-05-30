# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
# SPDX-FileCopyrightText: 2021 Adriaan de Groot <groot@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Applications version, managed by release script.
set(RELEASE_SERVICE_VERSION_MAJOR "23")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(NeoChat VERSION ${RELEASE_SERVICE_VERSION})

set(KF_MIN_VERSION "5.105.0")
set(QT_MIN_VERSION "5.15.2")
if (ANDROID)
    set(QT_MIN_VERSION "5.15.8")
endif()

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KDE_COMPILERSETTINGS_LEVEL 5.84)

include(FeatureSummary)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(ECMFindQmlModule)
include(KDECMakeSettings)
include(ECMAddTests)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(KDEGitCommitHooks)
include(ECMCheckOutboundLicense)
include(ECMQtDeclareLoggingCategory)
if (NOT ANDROID)
    include(KDEClangFormat)
endif()

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX NEOCHAT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/neochat-version.h
)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} NO_MODULE COMPONENTS Core Quick Gui QuickControls2 Multimedia Svg)
set_package_properties(Qt${QT_MAJOR_VERSION} PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} COMPONENTS Kirigami2 I18n Notifications Config CoreAddons Sonnet ItemModels)
set_package_properties(KF${QT_MAJOR_VERSION} PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
set_package_properties(KF${QT_MAJOR_VERSION}Kirigami2 PROPERTIES
        TYPE REQUIRED
        PURPOSE "Kirigami application UI framework"
        )
find_package(KF${QT_MAJOR_VERSION}KirigamiAddons 0.7.2 REQUIRED)

find_package(Qt${QT_MAJOR_VERSION}Keychain)
set_package_properties(Qt${QT_MAJOR_VERSION}Keychain PROPERTIES
    TYPE REQUIRED
    PURPOSE "Secure storage of account secrets"
)


find_package(ICU 61.0 COMPONENTS uc)
set_package_properties(ICU PROPERTIES
    TYPE REQUIRED
    PURPOSE "Unicode library"
)

if(ANDROID)
    find_package(OpenSSL)
    set_package_properties(OpenSSL PROPERTIES
        TYPE REQUIRED
        PURPOSE "Encrypted communications"
    )
else()
    find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} COMPONENTS Widgets)
    find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS QQC2DesktopStyle ConfigWidgets KIO WindowSystem)
    set_package_properties(KF${QT_MAJOR_VERSION}QQC2DesktopStyle PROPERTIES
        TYPE RUNTIME
    )
    ecm_find_qmlmodule(org.kde.syntaxhighlighting 1.0)
endif()

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    find_package(KF${QT_MAJOR_VERSION}DBusAddons ${KF_MIN_VERSION} REQUIRED)
endif()

if(QT_MAJOR_VERSION STREQUAL "6" AND NOT ANDROID AND NOT WIN32)
    set(QUOTIENT_SUFFIX "Qt6")
endif()

find_package(Quotient${QUOTIENT_SUFFIX} 0.6)
set_package_properties(Quotient${QUOTIENT_SUFFIX} PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Qt wrapper around Matrix API"
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
ecm_find_qmlmodule(org.kde.quickcharts 1.0)

find_package(KQuickImageEditor COMPONENTS)
set_package_properties(KQuickImageEditor PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Simple image editor for QtQuick applications"
    URL "https://invent.kde.org/libraries/kquickimageeditor/"
    PURPOSE "Add image editing capability to image attachments"
)

find_package(QCoro${QT_MAJOR_VERSION} 0.4 COMPONENTS Core REQUIRED)

qcoro_enable_coroutines()

find_package(KF${QT_MAJOR_VERSION}DocTools ${KF_MIN_VERSION})
set_package_properties(KF${QT_MAJOR_VERSION}DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)

if(NOT Quotient${QUOTIENT_SUFFIX}_VERSION_MINOR GREATER 6)
    cmake_policy(SET CMP0063 OLD)
endif()

if(ANDROID)
    find_package(Sqlite3)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/android/version.gradle.in ${CMAKE_BINARY_DIR}/version.gradle)
endif()

ki18n_install(po)

install(FILES org.kde.neochat.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.neochat.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.neochat.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES org.kde.neochat.tray.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

add_definitions(-DQT_NO_FOREACH)

add_subdirectory(src)
if (BUILD_TESTING AND Quotient${QUOTIENT_SUFFIX}_VERSION_MINOR GREATER 6)
    find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} NO_MODULE COMPONENTS Test)
    add_subdirectory(autotests)
endif()

if(KF${QT_MAJOR_VERSION}DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

if (NOT ANDROID)
    file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
    kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

    kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
endif()
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h *.qml)
# CI installs dependency headers to _install and _build, which break the reuse check
# Fixes the test by excluding this directory
list(FILTER ALL_SOURCE_FILES EXCLUDE REGEX [[_(install|build)/.*]])
ecm_check_outbound_license(LICENSES GPL-3.0-only FILES ${ALL_SOURCE_FILES})
