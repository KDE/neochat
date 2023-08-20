<!--
    SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carlschwan@kde.org>
    SPDX-FileCopyrightText: 2020-2021 Tobias Fella <tobias.fella@kde.org>
    SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
    SPDX-License-Identifier: CC0-1.0
-->

# NeoChat

A Qt/QML based Matrix client.

<a href='https://matrix.org'><img src='https://matrix.org/docs/legacy/made-for-matrix.png' alt='Made for Matrix' height=64 target=_blank /></a>
<a href='https://flathub.org/apps/details/org.kde.neochat'><img width='190px' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-i-en.png'/></a>

## Introduction

NeoChat is a client for [Matrix](https://matrix.org), the decentralized communication protocol for instant
messaging. It is a fork of Spectral, using KDE frameworks, most notably [Kirigami](https://invent.kde.org/frameworks/kirigami)
to provide a convergent experience across multiple platforms.

NeoChat also make use of other KDE Frameworks as well as [libQuotient](https://github.com/quotient-im/libQuotient), a
Qt-based SDK for the [Matrix Protocol](https://spec.matrix.org/).

![Timeline](https://cdn.kde.org/screenshots/neochat/application.png)

## Features

NeoChat aims to be a fully featured application for the Matrix specification. As such everything in the current stable specification with the notable exceptions
of VoIP, threads and some aspects of End-to-End Encryption are supported. There are a few other smaller omissions due to the fact that the Matrix spec is constantly
evolving but the aim remains to provide eventual support for the entire spec.

Due to the nature of the Matrix specification development NeoChat also supports numerous unstable features. Currently these are:
- Polls - MSC3381
- Sticker Packs - MSC2545
- Location Events - MSC3488

## Get it

Details where to find stable releases for NeoChat can be found on its [homepage](https://apps.kde.org/neochat).

In addition to the stable builds, unstable nightly builds are available for all platforms. These can be downloaded
from the [binary factory](https://binary-factory.kde.org/). There are unstable versions for the following platforms
in addition to stable ones:
- Android
- MacOS
- Windows

Additionally the nightly Flatpak version can be obtained from the nightly Flatpak repo using the following commands in your terminal:

```
flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
flatpak remote-add --if-not-exists kdeapps --from https://distribute.kde.org/kdeapps.flatpakrepo
flatpak install kdeapps org.kde.neochat
```

The unstable Android version can also be obtained from the [KDE nightly F-Droid repo](https://community.kde.org/Android/FDroid).

## Running

Just start the executable in your preferred way - either from the build directory or from the installed location.

## Building NeoChat

The best way to build KDE apps during development is to use `kdesrc-build`. The full instructions for this can be found on
the KDE community website's get involved section under [development](https://community.kde.org/Get_Involved/development). This
is primarily aimed at Linux development.

For Windows and Android [Craft](https://invent.kde.org/packaging/craft) is the primary choice. There are guides for setting up
development environments for [Windows](https://community.kde.org/Get_Involved/development/Windows) and [Android](https://develop.kde.org/docs/packaging/android/building_applications/).

## Tests

Tests are in the repository under [autotests](autotests) and should all pass for any contribution.

The project has CI setup to test new commits to the repository. All tests are expected to pass for a merge request to
be complete.

Current build status

![coverage](https://invent.kde.org/network/neochat/badges/master/pipeline.svg)

Currently the number of tests is limited, but growing. If anyone wants to help improve this, those
contributions would be especially welcome.

## Contributing

As is the case throughout the KDE ecosystem contributions are welcome from all. The code base is managed in the
[NeoChat repository](https://invent.kde.org/network/neochat) of the KDE Gitlab instance.

- [Code of Conduct](https://kde.org/code-of-conduct)
- [Report a Bug](https://bugs.kde.org/enter_bug.cgi?format=guided&product=neochat)
- [Feature Request](https://community.kde.org/Infrastructure/GitLab#Submitting_a_merge_request)
- [Create a Merge Request](https://community.kde.org/Infrastructure/GitLab#Submitting_a_merge_request)
- [Translation](https://community.kde.org/Get_Involved/translation)

## Contact

The best place to reach the maintainers is on the KDE Matrix instance in the NeoChat channel, [#neochat:kde.org](https://matrix.to/#/#neochat:kde.org).

## Acknowledgement

This program utilizes [libQuotient](https://github.com/quotient-im/libQuotient/) as its Matrix SDK.

This program is a fork of [Spectral](https://gitlab.com/spectral-im/spectral/).

## License

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)

This program is licensed under GNU General Public License, Version 3. 

