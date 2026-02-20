# SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carl@carlschwan.eu>
# SPDX-FileCopyrightText: 2020-2021 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
# SPDX-FileCopyrightText: 2021 Adriaan de Groot <groot@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

cmake_minimum_required(VERSION 3.16)

# KDE Applications version, managed by release script.
set(RELEASE_SERVICE_VERSION_MAJOR "26")
set(RELEASE_SERVICE_VERSION_MINOR "03")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(NeoChat VERSION ${RELEASE_SERVICE_VERSION})

set(KF_MIN_VERSION "6.17")
set(QT_MIN_VERSION "6.9")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(KDE_COMPILERSETTINGS_LEVEL 6.17)

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
include(ECMQmlModule)
include(ECMDeprecationSettings)
include(GenerateExportHeader)
include(ECMGenerateHeaders)
if (NOT ANDROID)
    include(KDEClangFormat)
endif()

set(QUOTIENT_FORCE_NAMESPACED_INCLUDES TRUE)

ecm_set_disabled_deprecation_versions(Qt 6.9.0 KF 6.17.0)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX NEOCHAT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/neochat-version.h
)

find_package(Qt6 ${QT_MIN_VERSION} NO_MODULE COMPONENTS Core Quick Gui QuickControls2 Multimedia Svg TextToSpeech WebView)
set_package_properties(Qt6 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)

if (QT_KNOWN_POLICY_QTP0004)
qt_policy(SET QTP0004 NEW)
endif ()

find_package(KF6 ${KF_MIN_VERSION} COMPONENTS Kirigami I18n Notifications Config CoreAddons Sonnet ItemModels ColorScheme IconThemes)
set_package_properties(KF6 PROPERTIES
    TYPE REQUIRED
    PURPOSE "Basic application components"
)
set_package_properties(KF6Kirigami PROPERTIES
    TYPE REQUIRED
    PURPOSE "Kirigami application UI framework"
)
find_package(KF6KirigamiAddons 1.10.0 REQUIRED)

if (UNIX AND NOT APPLE AND NOT ANDROID AND NOT NEOCHAT_FLATPAK AND NOT NEOCHAT_APPIMAGE)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS Purpose)
endif ()

if(ANDROID)
    find_package(OpenSSL)
    set_package_properties(OpenSSL PROPERTIES
        TYPE REQUIRED
        PURPOSE "Encrypted communications"
    )
else()
    find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS Widgets)
    find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS QQC2DesktopStyle KIO WindowSystem StatusNotifierItem)
    find_package(KF6SyntaxHighlighting ${KF_MIN_VERSION} REQUIRED)
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

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE AND NOT HAIKU)
    find_package(KF6DBusAddons ${KF_MIN_VERSION} REQUIRED)
endif()

find_package(QuotientQt6 0.9.5)
set_package_properties(QuotientQt6 PROPERTIES
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

option(WITH_UNIFIEDPUSH "Build with KUnifiedPush support" ON)

if (ANDROID OR APPLE OR WIN32 OR HAIKU)
    set(WITH_UNIFIEDPUSH OFF)
endif()

if (WITH_UNIFIEDPUSH)
    find_package(KUnifiedPush)
    set_package_properties(KUnifiedPush PROPERTIES
        TYPE REQUIRED
        PURPOSE "Push notification support"
        URL "https://invent.kde.org/libraries/kunifiedpush"
    )
endif()

if(ANDROID)
    find_package(Sqlite3)
    set(BUILD_TESTING FALSE)
endif()

ki18n_install(po)

install(FILES org.kde.neochat.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.neochat.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.neochat.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES org.kde.neochat.tray.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

add_definitions(-DQT_NO_FOREACH)

add_subdirectory(src)

if (BUILD_TESTING)
    find_package(Qt6 ${QT_MIN_VERSION} NO_MODULE COMPONENTS Test HttpServer QuickTest)
    add_subdirectory(autotests)
    # add_subdirectory(appiumtests)
    if (NOT ANDROID)
        add_subdirectory(memorytests)
    endif()
endif()

if(KF6DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

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
