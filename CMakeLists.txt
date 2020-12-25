cmake_minimum_required(VERSION 3.1)

project(Neochat)

set(KF5_MIN_VERSION "5.76.0")
set(QT_MIN_VERSION "5.15.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(ECMQMLModules)
include(KDEClangFormat)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

if(NEOCHAT_FLATPAK)
    include(cmake/Flatpak.cmake)
endif()

# Fix a crash due to problems with quotient's event system. Can probably be removed once the reworked event system is in
cmake_policy(SET CMP0063 OLD)

ecm_setup_version(1.0.80
    VARIABLE_PREFIX NEOCHAT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/neochat-version.h
)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Core Quick Gui QuickControls2 Multimedia Svg)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS Kirigami2 I18n Notifications Config CoreAddons)

if(ANDROID)
    find_package(OpenSSL REQUIRED)
else()
    find_package(Qt5Keychain REQUIRED)
    find_package(KF5DBusAddons ${KF5_MIN_VERSION} REQUIRED)
endif()

find_package(Quotient 0.7)
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

find_package(KQuickImageEditor COMPONENTS)
set_package_properties(KQuickImageEditor PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Simple image editor for QtQuick applications"
    URL "https://invent.kde.org/libraries/kquickimageeditor/"
    PURPOSE "Add image editing capability to image attachments"
)

install(FILES org.kde.neochat.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.neochat.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.neochat.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES org.kde.neochat-symbolic.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/16x16/apps RENAME org.kde.neochat.svg)
install(FILES org.kde.neochat-symbolic.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/16x16@2/apps RENAME org.kde.neochat.svg)
install(FILES org.kde.neochat-symbolic.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/16x16@3/apps RENAME org.kde.neochat.svg)
install(FILES org.kde.neochat-symbolic.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/symbolic/apps)
install(FILES neochat.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})

# add_definitions(-DQT_NO_KEYWORDS) Need to fix libQuotient first
add_definitions(-DQT_NO_FOREACH)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
