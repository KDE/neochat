# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
# SPDX-FileCopyrightText: 2021 Adriaan de Groot <groot@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Applications version, managed by release script.
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "04")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(NeoChat VERSION ${RELEASE_SERVICE_VERSION})

set(KF_MIN_VERSION "5.240.0")
set(QT_MIN_VERSION "6.5")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KDE_COMPILERSETTINGS_LEVEL 5.105)

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
include(ECMAddAndroidApk)
if (NOT ANDROID)
    include(KDEClangFormat)
endif()

if(NEOCHAT_FLATPAK)
    include(cmake/Flatpak.cmake)
endif()

set(QUOTIENT_FORCE_NAMESPACED_INCLUDES TRUE)

set(RECOMMENDED_SPACE_ID "" CACHE STRING "The matrix ID of a space that will be recommended to the user. Only relevant to distros")
set(RECOMMENDED_SPACE_DISPLAYNAME "" CACHE STRING "The displayname of a space that will be recommended to the user. Only relevant to distros")
set(RECOMMENDED_SPACE_AVATAR "" CACHE STRING "mxc url of a space that will be recommended to the user. Only relevant to distros")
set(RECOMMENDED_SPACE_DESCRIPTION "" CACHE STRING "Description of a space that will be recommended to the user. Only relevant to distros")

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX NEOCHAT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/neochat-version.h
)

find_package(Qt6 ${QT_MIN_VERSION} NO_MODULE COMPONENTS Core Quick Gui QuickControls2 Multimedia Svg WebView)
set_package_properties(Qt6 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
find_package(KF6 ${KF_MIN_VERSION} COMPONENTS Kirigami I18n Notifications Config CoreAddons Sonnet ItemModels ColorScheme)
set_package_properties(KF6 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
set_package_properties(KF6Kirigami PROPERTIES
    TYPE REQUIRED
    PURPOSE "Kirigami application UI framework"
)
find_package(KF6KirigamiAddons 0.7.2 REQUIRED)

if(ANDROID)
    find_package(OpenSSL)
    set_package_properties(OpenSSL PROPERTIES
        TYPE REQUIRED
        PURPOSE "Encrypted communications"
    )
else()
    find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS Widgets)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS QQC2DesktopStyle KIO WindowSystem StatusNotifierItem)
    set_package_properties(KF6QQC2DesktopStyle PROPERTIES
        TYPE RUNTIME
    )
    ecm_find_qmlmodule(org.kde.syntaxhighlighting 1.0)

    find_package(ICU 61.0 COMPONENTS uc)
    set_package_properties(ICU PROPERTIES
        TYPE REQUIRED
        PURPOSE "Unicode library"
    )
endif()

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE)
    find_package(KF6DBusAddons ${KF_MIN_VERSION} REQUIRED)
endif()

find_package(QuotientQt6 0.7)
set_package_properties(QuotientQt6 PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Qt wrapper around Matrix API"
    URL "https://github.com/quotient-im/libQuotient/"
    PURPOSE "Talk with matrix server"
)

if (NOT TARGET Olm::Olm)
    message(FATAL_ERROR "NeoChat requires Quotient with the E2EE feature enabled")
endif()


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
ecm_find_qmlmodule(QtLocation)
ecm_find_qmlmodule(org.kde.prison)

find_package(KQuickImageEditor COMPONENTS)
set_package_properties(KQuickImageEditor PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Simple image editor for QtQuick applications"
    URL "https://invent.kde.org/libraries/kquickimageeditor/"
    PURPOSE "Add image editing capability to image attachments"
)

find_package(QCoro6 0.4 COMPONENTS Core Network REQUIRED)

qcoro_enable_coroutines()

find_package(KF6DocTools ${KF_MIN_VERSION})
set_package_properties(KF6DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)

find_package(KUnifiedPush QUIET)
set_package_properties(KUnifiedPush PROPERTIES
    TYPE OPTIONAL
    PURPOSE "Push notification support"
    URL "https://invent.kde.org/libraries/kunifiedpush"
)

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

if (BUILD_TESTING)
    find_package(Qt6 ${QT_MIN_VERSION} NO_MODULE COMPONENTS Test)
    add_subdirectory(autotests)
    add_subdirectory(appiumtests)
endif()

if(KF6DocTools_FOUND)
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

ecm_qt_install_logging_categories(
        EXPORT NEOCHAT
        FILE neochat.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )
