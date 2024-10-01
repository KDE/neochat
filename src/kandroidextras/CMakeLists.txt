# SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(KAndroidExtras STATIC)

target_sources(KAndroidExtras PRIVATE
    android/activity.cpp
    android/android_headers.cpp
    android/contentresolver.cpp
    android/context.cpp
    android/intent.cpp
    android/uri.cpp

    java/java_headers.cpp
    java/javalocale.cpp

    jni/jni_headers.cpp
)

if (NOT ANDROID)
    target_sources(KAndroidExtras PRIVATE
        fake/mock_impl.cpp
        fake/mock_jniobject.cpp
    )
endif()

generate_export_header(KAndroidExtras BASE_NAME KAndroidExtras)
target_compile_features(KAndroidExtras PUBLIC cxx_std_20) # for __VA_OPT__
target_link_libraries(KAndroidExtras PUBLIC Qt::Core)

if (ANDROID)
    target_link_libraries(KAndroidExtras PRIVATE Qt::CorePrivate)
else()
    target_include_directories(KAndroidExtras PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/fake ${CMAKE_CURRENT_SOURCE_DIR}/fake/qt6)
endif()

ecm_generate_headers(KAndroidExtras_android_FORWARDING_HEADERS
    HEADER_NAMES
        Activity
        AndroidTypes
        CalendarContract
        ContentResolver
        Context
        Intent
        ManifestPermission
        OpenableColumns
        Settings
        Uri
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_android_HEADERS
    RELATIVE android
)

ecm_generate_headers(KAndroidExtras_java_FORWARDING_HEADERS
    HEADER_NAMES
        JavaLocale
        JavaTypes
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_java_HEADERS
    RELATIVE java
)

ecm_generate_headers(KAndroidExtras_jni_FORWARDING_HEADERS
    HEADER_NAMES
        JniArgument
        JniArray
        JniObject
        JniMethod
        JniReturnValue
        JniPp
        JniProperty
        JniSignature
        JniTypes
        JniTypeTraits
    PREFIX KAndroidExtras
    REQUIRED_HEADERS KAndroidExtras_jni_HEADERS
    RELATIVE jni
)
