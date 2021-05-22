<!--
    SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carlschwan@kde.org>
    SPDX-FileCopyrightText: 2020-2021 Tobias Fella <fella@posteo.de>
    SPDX-License-Identifier: CC0-1.0
-->
# NeoChat

NeoChat is a client for Matrix, the decentralized communication protocol for instant
messaging. It is a fork of Spectral, using KDE frameworks, most notably Kirigami,
KConfig and KI18n.

<a href='https://flathub.org/apps/details/org.kde.neochat'><img width='190px' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-i-en.png'/></a>


## Get it

A stable release [is available](https://apps.kde.org/en/neochat) for download for Linux distributions.


Along with the stable release, a Flatpak version is available for the nightly
version:

```
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak remote-add --if-not-exists kdeapps --from https://distribute.kde.org/kdeapps.flatpakrepo
flatpak install kdeapps org.kde.neochat
```

A nightly build is also available for Android in the [KDE nightly F-Droid repo](https://community.kde.org/Android/FDroid)
and can also directly be downloaded from the [binary factory](https://binary-factory.kde.org/view/Android/job/Neochat_android/).

Nightly builds for [Windows](https://binary-factory.kde.org/job/NeoChat_Nightly_win64/), [MacOS](https://binary-factory.kde.org/job/NeoChat_Nightly_macos/) and [AppImages](https://binary-factory.kde.org/job/NeoChat_Nightly_appimage/) can also be downloaded from the [binary factory](https://binary-factory.kde.org/search/?q=neochat).

![Timeline](https://invent.kde.org/websites/product-screenshots/-/raw/master/neochat/application.png)

## Features

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

