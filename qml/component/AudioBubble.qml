import QtQuick 2.9
import QtQuick.Controls 2.2
import QtMultimedia 5.8

AvatarContainer {
    Button {
        id: downloadButton

        text: content.body
        highlighted: !sentByMe
        flat: true

        onClicked: downloadable.downloadAndOpen()

        Audio {
            audioRole: Audio.VoiceCommunicationRole
        }
    }
}
