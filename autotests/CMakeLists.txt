# SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
# SPDX-License-Identifier: BSD-2-Clause

enable_testing()

add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" )

ecm_add_test(
    neochatroomtest.cpp
    LINK_LIBRARIES neochat Qt::Test
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
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME chatbarcachetest
)

ecm_add_test(
    chatdocumenthandlertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME chatdocumenthandlertest
)

ecm_add_test(
    messageeventmodeltest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME messageeventmodeltest
)

ecm_add_test(
    actionshandlertest.cpp
    LINK_LIBRARIES neochat Qt::Test
    TEST_NAME actionshandlertest
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
