/*
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import QtTest

TestCase {
    id: root

    name: "PaddingTest"
    visible: true
    when: windowShown

    width: 300
    height: 300

    Component {
        id: invalidMxcUrlComponent
        Image {
            // Missing user_id, which is required in libQuotient
            source: "mxc://localhost:1234/AAAAAAAAAAAAAAAAAAAAAAAA?room_id=!AjYwbldYDmSVfGrVHV:pyra.sh&event_id=$vJfWLoXK02im0M3rlFWLosiHojrwWSknLb0JXveEE1o"
        }
    }

    Component {
        id: validMxcUrlComponent
        Image {
            source: "mxc://localhost:1234/AAAAAAAAAAAAAAAAAAAAAAAA?user_id=@user:localhost:1234&room_id=!AjYwbldYDmSVfGrVHV:pyra.sh&event_id=$vJfWLoXK02im0M3rlFWLosiHojrwWSknLb0JXveEE1o"
        }
    }

    // TODO: use ImageComponent directly

    function test_invalid() {
        const item = createTemporaryObject(invalidMxcUrlComponent, this);
        verify(item);
        compare(item.status, Image.Loading);

        // It should fail if we didn't specify the connection
        ignoreWarning("No connection specified, cannot convert mxc request");
        tryCompare(item, "status", Image.Error);
    }

    function test_basic() {
        const item = createTemporaryObject(validMxcUrlComponent, this);
        verify(item);
        compare(item.status, Image.Loading); // initial load
        tryCompare(item, "status", Image.Error);
    }
}
