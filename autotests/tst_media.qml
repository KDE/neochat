/*
 *  SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtTest

import org.kde.kirigami as Kirigami
import org.kde.neochat.messagecontent

TestCase {
    id: root

    name: "ImageComponentTest"

    width: 300
    height: 300

    // Base component to not re-initialize the same variables over and over
    component BaseImageComponent: ImageComponent {
        eventId: "dummyevent"
        display: "dummytext"
        fileTransferInfo: null
    }

    Component {
        id: invalidMxcUrlComponent
        BaseImageComponent {
            componentAttributes: QtObject {
                property bool isSticker: false
                property bool animated: false
                // Missing user_id, which is required in libQuotient
                property string source: "mxc://localhost:1234/AAAAAAAAAAAAAAAAAAAAAAAA?room_id=!AjYwbldYDmSVfGrVHV:localhost&event_id=$vJfWLoXK02im0M3rlFWLosiHojrwWSknLb0JXveEE1o"
            }
        }
    }

    Component {
        id: validMxcUrlComponent
        BaseImageComponent {
            componentAttributes: QtObject {
                property bool isSticker: false
                property bool animated: false
                property string source: "mxc://localhost:1234/AAAAAAAAAAAAAAAAAAAAAAAA?user_id=@user:localhost:1234&room_id=!AjYwbldYDmSVfGrVHV:localhost&event_id=$vJfWLoXK02im0M3rlFWLosiHojrwWSknLb0JXveEE1o"
            }
        }
    }

    function test_invalid() {
        wait(1000); // Wait for Quotient to grab the right capability

        ignoreWarning("No connection specified, cannot convert mxc request");

        const item = createTemporaryObject(invalidMxcUrlComponent, this);
        verify(item);
        compare(item._private.imageItem.status, Image.Loading);

        // It should fail if we didn't specify the connection
        tryCompare(item._private.imageItem, "status", Image.Error);
    }

    function test_basic() {
        wait(1000); // Wait for Quotient to grab the right capability

        const item = createTemporaryObject(validMxcUrlComponent, this);
        verify(item);
        compare(item._private.imageItem.status, Image.Loading); // initial load
        tryCompare(item._private.imageItem, "status", Image.Error);
    }
}
