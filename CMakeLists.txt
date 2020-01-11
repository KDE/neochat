CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(IDENTIFIER "org.eu.encom.spectral")
set(COPYRIGHT "Copyright © 2018-2019 bhat@encom.eu.org")

project(spectral VERSION 0.0.0 LANGUAGES CXX)

if(UNIX AND NOT APPLE)
    set(LINUX 1)
endif(UNIX AND NOT APPLE)

include(CheckCXXCompilerFlag)
if (NOT WIN32)
    include(GNUInstallDirs)
    include(cmake/ECMInstallIcons.cmake)
endif(NOT WIN32)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 17)

# Setup command line parameters for the compiler and linker
foreach (FLAG "" all pedantic extra no-unused-parameter)
    CHECK_CXX_COMPILER_FLAG("-W${FLAG}" WARN_${FLAG}_SUPPORTED)
    if ( WARN_${FLAG}_SUPPORTED AND NOT CMAKE_CXX_FLAGS MATCHES "(^| )-W?${FLAG}($| )")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W${FLAG}")
    endif ()
endforeach ()

# Find the libraries
find_package(Qt5 5.12 REQUIRED Widgets Network Quick Qml Gui Svg Multimedia QuickControls2)
if(LINUX)
    find_package(Qt5DBus REQUIRED)
endif(LINUX)
if (APPLE)
    find_package(Qt5MacExtras REQUIRED)
endif(APPLE)
# Qt5_Prefix is only used to show Qt path in message()
# Qt5_BinDir is where all the binary tools for Qt are
if (QT_QMAKE_EXECUTABLE)
    get_filename_component(Qt5_BinDir "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    get_filename_component(Qt5_Prefix "${Qt5_BinDir}/.." ABSOLUTE)
else()
    get_filename_component(Qt5_BinDir "${Qt5_DIR}/../../../bin" ABSOLUTE)
    get_filename_component(Qt5_Prefix "${Qt5_DIR}/../../../.." ABSOLUTE)
endif()

# prevent error "You must build your code with position independent code if Qt was built with..
if (Qt5_POSITION_INDEPENDENT_CODE)
  SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

set(QML_IMPORT_PATH ${CMAKE_SOURCE_DIR}/qml ${CMAKE_SOURCE_DIR}/imports CACHE string "" FORCE)

if(WIN32)
    enable_language(RC)
    include(CMakeDetermineRCCompiler)
endif()

if ((NOT DEFINED USE_INTREE_LIBQMC OR USE_INTREE_LIBQMC)
        AND EXISTS ${PROJECT_SOURCE_DIR}/include/libQuotient/lib/util.h)
    add_subdirectory(include/libQuotient EXCLUDE_FROM_ALL)
    include_directories(include/libQuotient)
    if (NOT DEFINED USE_INTREE_LIBQMC)
        set (USE_INTREE_LIBQMC 1)
    endif ()
endif ()
if (NOT USE_INTREE_LIBQMC)
    find_package(Quotient 0.6 REQUIRED)
    if (NOT Quotient_FOUND)
        message( WARNING "libQuotient not found; configuration will most likely fail.")
    endif ()
endif ()

find_package(Qt5Keychain REQUIRED)

find_package(cmark REQUIRED)

add_subdirectory(include/SortFilterProxyModel EXCLUDE_FROM_ALL)

message( STATUS )
message( STATUS "=============================================================================" )
message( STATUS "                          Spectral Build Information" )
message( STATUS "=============================================================================" )
if (CMAKE_BUILD_TYPE)
    message( STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif(CMAKE_BUILD_TYPE)
message( STATUS "Spectral install prefix: ${CMAKE_INSTALL_PREFIX}" )
# Get Git info if possible
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND
        "${GIT_EXECUTABLE}" rev-parse -q HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    message( STATUS "Git SHA1: ${GIT_SHA1}")
endif()
message( STATUS "Using compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}" )
message( STATUS "Using Qt ${Qt5_VERSION} at ${Qt5_Prefix}" )
if (USE_INTREE_LIBQMC)
    message( STATUS "Using in-tree libQuotient")
    if (GIT_FOUND)
        execute_process(COMMAND
            "${GIT_EXECUTABLE}" rev-parse -q HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/libQuotient
            OUTPUT_VARIABLE LIB_GIT_SHA1
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        message( STATUS "  Library git SHA1: ${LIB_GIT_SHA1}")
    endif (GIT_FOUND)
else ()
    message( STATUS "Using libQuotient ${Quotient_VERSION} at ${Quotient_DIR}")
endif ()

message( STATUS "=============================================================================" )
message( STATUS )

# Set up source files
set(spectral_SRCS
    src/notifications/manager.h
    src/accountlistmodel.h
    src/controller.h
    src/emojimodel.h
    src/imageclipboard.h
    src/matriximageprovider.h
    src/messageeventmodel.h
    src/roomlistmodel.h
    src/spectralroom.h
    src/spectraluser.h
    src/trayicon.h
    src/userlistmodel.h
    src/publicroomlistmodel.h
    src/userdirectorylistmodel.h
    src/utils.h
    src/accountlistmodel.cpp
    src/controller.cpp
    src/emojimodel.cpp
    src/imageclipboard.cpp
    src/matriximageprovider.cpp
    src/messageeventmodel.cpp
    src/roomlistmodel.cpp
    src/spectralroom.cpp
    src/spectraluser.cpp
    src/trayicon.cpp
    src/userlistmodel.cpp
    src/publicroomlistmodel.cpp
    src/userdirectorylistmodel.cpp
    src/utils.cpp
    src/main.cpp
)

if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Foundation -framework Cocoa")
    set(spectral_SRCS ${spectral_SRCS} src/notifications/managermac.mm)
elseif (WIN32)
    set(spectral_SRCS ${spectral_SRCS} src/notifications/managerwin.cpp src/notifications/wintoastlib.cpp)
else ()
    set(spectral_SRCS ${spectral_SRCS} src/notifications/managerlinux.cpp)
endif ()

set(spectral_QRC
    res.qrc
)

QT5_ADD_RESOURCES(spectral_QRC_SRC ${spectral_QRC})
set_property(SOURCE qrc_resources.cpp PROPERTY SKIP_AUTOMOC ON)

if(WIN32)
    set(spectral_WINRC spectral_win32.rc)
    set_property(SOURCE spectral_win32.rc APPEND PROPERTY
        OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/icons/icon.ico
    )
endif()

if(APPLE)
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})

    set(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})

    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${spectral_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${spectral_VERSION})

    set(ICON_NAME "icon.icns")
    set(${PROJECT_NAME}_MAC_ICON "${PROJECT_SOURCE_DIR}/icons/${ICON_NAME}")
    set(MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
    set_property(SOURCE "${${PROJECT_NAME}_MAC_ICON}" PROPERTY
        MACOSX_PACKAGE_LOCATION Resources)
endif(APPLE)

# Windows, this is a GUI executable; OSX, make a bundle
add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
    ${spectral_SRCS} ${spectral_QRC_SRC} $<TARGET_OBJECTS:SortFilterProxyModel>
    ${spectral_WINRC} ${${PROJECT_NAME}_MAC_ICON})

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets Qt5::Quick Qt5::Qml Qt5::Gui Qt5::Network Qt5::Svg Qt5::QuickControls2
    Quotient
    cmark::cmark
    ${QTKEYCHAIN_LIBRARIES}
    )
target_compile_definitions(${PROJECT_NAME} PRIVATE
    GIT_SHA1="${GIT_SHA1}" LIB_GIT_SHA1="${LIB_GIT_SHA1}")

if (APPLE)
    target_link_libraries(${PROJECT_NAME} Qt5::MacExtras)
elseif(LINUX)
    target_link_libraries(${PROJECT_NAME} Qt5::DBus)
endif()

# macOS specific config for bundling
set_property(TARGET ${PROJECT_NAME} PROPERTY
    MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/macOS/Info.plist.in")

# Installation

if (NOT CMAKE_INSTALL_BINDIR)
    set(CMAKE_INSTALL_BINDIR ".")
endif()

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR})
if(LINUX)
    install(FILES linux/${IDENTIFIER}.desktop
            DESTINATION  ${CMAKE_INSTALL_DATADIR}/applications
            )
    install(FILES linux/${IDENTIFIER}.appdata.xml
            DESTINATION  ${CMAKE_INSTALL_DATADIR}/metainfo
            )
    file(GLOB spectral_icons icons/hicolor/*-apps-org.eu.encom.spectral.png)
    ecm_install_icons(ICONS ${spectral_icons}
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons
    )
endif(LINUX)
