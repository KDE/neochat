<!--
    SPDX-FileCopyrightText: 2020-2021 Carl Schwan <carlschwan@kde.org>
    SPDX-FileCopyrightText: 2020-2024 Tobias Fella <tobias.fella@kde.org>
    SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
    SPDX-License-Identifier: CC0-1.0
-->

# NeoChat

A Qt/QML based Matrix client.

<a href='https://matrix.org'><img src='https://matrix.org/docs/legacy/made-for-matrix.png' alt='Made for Matrix' height=64 target=_blank /></a>
<a href='https://flathub.org/apps/details/org.kde.neochat'><img width='190px' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-i-en.png'/></a>
<a href='https://snapcraft.io/neochat'><img width='190px' alt='Download on the Snap Store' src='https://apps.kde.org/store_badges/snapstore/en.svg'/></a>

## Introduction

NeoChat is a client for [Matrix](https://matrix.org), the decentralized communication protocol for instant
messaging.

NeoChat is based on KDE frameworks and as [libQuotient](https://github.com/quotient-im/libQuotient), a
Qt-based SDK for the [Matrix Protocol](https://spec.matrix.org/).

![Timeline](https://cdn.kde.org/screenshots/neochat/application.png)

## Features

NeoChat aims to be a fully featured application for the Matrix specification. As such, most parts of the current specification are supported, with the notable exceptions
of VoIP, threads, and some aspects of End-to-End Encryption. There are a few other smaller omissions due to the Matrix spec constantly
evolving, but the aim remains to provide eventual support for the entire spec.

## Get it

Details where to find stable releases for NeoChat can be found on its [homepage](https://apps.kde.org/neochat).

Nightly builds for Linux and Windows can be downloaded from [cdn.kde.org](https://cdn.kde.org/ci-builds/network/neochat/).
Nightly builds for Android are available from [KDE's nightly F-Droid repository](https://community.kde.org/Android/F-Droid).
Nightly Flatpaks are available from [KDE's nightly Flatpak repository](https://userbase.kde.org/Tutorials/Flatpak).

## Building NeoChat

The best way to build KDE apps during development is to use `kdesrc-build`. The full instructions for this can be found on
the KDE community website's get involved section under [development](https://community.kde.org/Get_Involved/development). This
is primarily aimed at Linux development.

For Windows and Android, [Craft](https://invent.kde.org/packaging/craft) is the primary choice. There are guides for setting up
development environments for [Windows](https://community.kde.org/Get_Involved/development/Windows) and [Android](https://develop.kde.org/docs/packaging/android/building_applications/).

## Running

Start the executable in your preferred way – either from the build directory or from the installed location.

## Tests

Tests are in the repository under [autotests](autotests) and [appiumtests](appiumtests).

The project has CI setup to test new commits to the repository. All tests are expected to pass for a merge request to
be complete.

## Current build status

![coverage](https://invent.kde.org/network/neochat/badges/master/pipeline.svg)

Currently, the number of tests is limited but growing. If anyone wants to help improve this, those
contributions would be especially welcome.

## Contributing

As is the case throughout the KDE ecosystem, contributions are welcome from all. The code base is managed in the
[NeoChat repository](https://invent.kde.org/network/neochat) of the KDE Gitlab instance.

- [Code of Conduct](https://kde.org/code-of-conduct)
- [Report a Bug](https://bugs.kde.org/enter_bug.cgi?format=guided&product=neochat)
- [Feature Request](https://community.kde.org/Infrastructure/GitLab#Submitting_a_merge_request)
- [Create a Merge Request](https://community.kde.org/Infrastructure/GitLab#Submitting_a_merge_request)
- [Translation](https://community.kde.org/Get_Involved/translation)

## Contact

The best place to reach the maintainers is on the KDE Matrix instance in the NeoChat channel, [#neochat:kde.org](https://go.kde.org/matrix/#/#neochat:kde.org). See [Matrix](https://community.kde.org/Matrix) for more details.

## Acknowledgement

NeoChat uses [libQuotient](https://github.com/quotient-im/libQuotient/) as its Matrix SDK.

NeoChat is a fork of [Spectral](https://gitlab.com/spectral-im/spectral/).

## License

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)

This program is licensed under GNU General Public License, Version 3. 

