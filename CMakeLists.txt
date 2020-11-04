cmake_minimum_required(VERSION 3.1)

project(Neochat)

set(KF5_MIN_VERSION "5.74.0")
set(QT_MIN_VERSION "5.15.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FeatureSummary)
include(ECMSetupVersion)
include(KDEInstallDirs)
include(KDEClangFormat)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

ecm_setup_version(0.1.0
    VARIABLE_PREFIX NEOCHAT
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/neochat-version.h
)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets Core Quick Gui QuickControls2 Multimedia Svg)
find_package(KF5 ${REQUIRED_KF5_VERSION} REQUIRED COMPONENTS Kirigami2 ItemModels I18n Notifications Config CoreAddons)

if(ANDROID)
    find_package(OpenSSL REQUIRED)
else()
    find_package(Qt5Keychain REQUIRED)
endif()

find_package(Quotient 0.7 REQUIRED)
find_package(cmark REQUIRED)

install(PROGRAMS org.kde.neochat.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.neochat.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES neochat.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)
install(FILES neochat.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})

# add_definitions(-DQT_NO_KEYWORDS) Need to fix libQuotient first
add_definitions(-DQT_NO_FOREACH)

add_subdirectory(src)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
