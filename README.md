# Neochat

Neochat is a client for Matrix, the decentralized communication protocol for instant
messaging. It is a fork of Spectral, using KDE frameworks, most notably Kirigami,
KConfig and KI18n.

<div style="display: flex; flex-wrap: wrap;">
<img src="https://www.plasma-mobile.org/img/post-2020-10/post-2020-10-neochat.png" style="max-width: 100%; height: auto;">
<img src="https://www.plasma-mobile.org/img/post-2020-10/post-2020-10-neochat-timeline.png" style="max-width: 100%; height: auto;">
<img src="https://www.plasma-mobile.org/img/post-2020-10/post-2020-10-neochat-explore.png" style="max-width: 100%; height: auto;">
</div>

# Features

* Sending messages
* Sending files from clipboard and filesystem
* Reply to message (right-click on a message to access menu)
* Start a private chat (but not encrypted)
* Show notifications, for the moment there is only a global switch
to disable it. We plan to implement the configuration part of the
specification soon.
* Autocompletion of usernames in chat
* Emoji picker
* Basic room setting page
* Send and accept invitations
* /rainbow <message> (very important)
* /me <message>

NeoChat is still missing a few features to become a full-featured
Matrix client (most notably encryption support and video chat support).
We welcome contributions in this direction.

## Contact

You can reach the maintainers at #neochat:kde.org, if you are already on Matrix.
Development happens in http://invent.kde.org/network/neochat (not in GitHub).

## Acknowledgement

This program utilizes [libQuotient](https://github.com/quotient-im/libQuotient/)
library and some C++ models from [Quaternion](https://github.com/quotient-im/Quaternion/).

This program is a fork of [Spectral](https://gitlab.com/spectral-im/spectral/).

## License

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)

This program is licensed under GNU General Public License, Version 3. 

