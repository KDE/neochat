# SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
# SPDX-License-Identifier: BSD-2-Clause

enable_testing()

add_library(neochat_server STATIC server.cpp)

target_link_libraries(neochat_server PUBLIC Qt::HttpServer QuotientQt6)

add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

ecm_add_test(
    neochatroomtest.cpp
    LINK_LIBRARIES neochat Qt::Test Qt::HttpServer neochat_server
    TEST_NAME neochatroomtest
)

ecm_add_test(
    texthandlertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME texthandlertest
)

ecm_add_test(
    delegatesizehelpertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME delegatesizehelpertest
)

ecm_add_test(
    mediasizehelpertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME mediasizehelpertest
)

ecm_add_test(
    eventhandlertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME eventhandlertest
)

ecm_add_test(
    chatbarcachetest.cpp
    LINK_LIBRARIES neochat Qt::Test Qt::HttpServer neochat_server
    TEST_NAME chatbarcachetest
)

ecm_add_test(
    timelinemessagemodeltest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME timelinemessagemodeltest
)

ecm_add_test(
    windowcontrollertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME windowcontrollertest
)

ecm_add_test(
    pollhandlertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME pollhandlertest
)

ecm_add_test(
    reactionmodeltest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME reactionmodeltest
)

ecm_add_test(
    linkpreviewertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME linkpreviewertest
)

ecm_add_test(
    messagecontentmodeltest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME messagecontentmodeltest
)

ecm_add_test(
    actionstest.cpp
    LINK_LIBRARIES neochat Qt::Test neochat_server
    TEST_NAME actionstest
)

ecm_add_test(
    servernoticestest.cpp
    LINK_LIBRARIES neochat Qt::Test neochat_server
    TEST_NAME servernoticestest
)

ecm_add_test(
    roommanagertest.cpp
    LINK_LIBRARIES neochat Qt::Test neochat_server
    TEST_NAME roommanagertest
)

ecm_add_test(
    modeltest.cpp
    LINK_LIBRARIES neochat Qt::Test neochat_server Devtools
    TEST_NAME modeltest
)

macro(add_qml_tests)
    if (WIN32)
        set(_extra_args -platform offscreen)
    endif()

    foreach(test ${ARGV})
        add_test(NAME ${test}
                COMMAND qmltest
                ${_extra_args}
                -input ${test}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endforeach()
endmacro()

add_executable(qmltest qmltest.cpp
    chatmarkdownhelpertestwrapper.h
    chattextitemhelpertestwrapper.h
)
qt_add_qml_module(qmltest URI NeoChatTestUtils)

target_link_libraries(qmltest
    PRIVATE
    Qt6::Qml
    Qt6::QuickTest
    LibNeoChat
    LibNeoChatplugin
)

add_qml_tests(
    chattextitemhelpertest.qml
    chatmarkdownhelpertest.qml
)
