# Matrique

[![Pipeline Status](https://gitlab.com/b0/matrique/badges/master/pipeline.svg)](https://gitlab.com/b0/matrique/commits/master)
[![Build Status](https://ci.appveyor.com/api/projects/status/idt149cdxwurbukh?svg=true)](https://ci.appveyor.com/project/BlackHat/matrique)
[![Coverage Report](https://gitlab.com/b0/matrique/badges/master/coverage.svg)](https://gitlab.com/b0/matrique/commits/master)

<a href='https://flathub.org/apps/details/org.eu.encom.matrique'><img width='240' alt='Get it on Flathub' src='https://flathub.org/assets/badges/flathub-badge-i-en.png'/></a>

> "Nobody can be told what the matrix is, you have to see it for yourself. "

Matrique is a glossy cross-platform client for Matrix, the decentralized communication protocol for instant messaging.

## Contact

You can reach the maintainer at #matrique:matrix.org, if you are already on Matrix.

Also, you can file an issue at this project if anything goes wrong.

## Install

### GNU/Linux

You can fetch the release of Matrique on Flathub. The link is at the beginning of this README.

Alternatively, you can download the Flatpak nightly build bundle from ![Gitlab CI](https://gitlab.com/b0/matrique/-/jobs/artifacts/dev/raw/flatpak/matrique.flatpak?job=build-flatpak).

Open the bundle using your preferred software center.

#### Requirements

An operating system with Flatpak installed.
Fedora 28 is known to work, other GNU/Linux distributions should work as well if they have Flatpak installed.

#### Run

If you install Matrique through Flatpak, you should already be able to run it from the app launcher.

If not, 

```bash
flatpak run org.eu.encom.matrique
```

### Windows

There are no tested builds on Windows yet, but you can fetch the nightly builds from ![AppVeyor](https://ci.appveyor.com/api/projects/BlackHat/matrique/artifacts/matrique.zip).

Most functions should work fine, but things are expected to go wrong. Open an issue if you find any bugs.

### MacOS

I suggest you to check out ![Seaglass](https://github.com/neilalexander/seaglass).

## Compile from Source

You can compile Matrique from source if you want to.

The requirements are Qt, QMake, Qt Quick, Qt Quick Controls 2, Qt Multimedia, and several others.

* Fetch the code.

```bash
git clone --recursive https://gitlab.com/b0/matrique
```

* Open in Qt Creator using the default config.
* Run.

*Still prefer commands?*

```bash
mkdir build && cd build
qmake ..
make
sudo make install
```

## Acknowledgement

This program uses libqmatrixclient library and C++ models from Quaternion. 

![Quaternion](https://github.com/QMatrixClient/Quaternion)

![libqmatrixclient](https://github.com/QMatrixClient/libqmatrixclient)

## License

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)

This program is licensed undedr GNU General Public License, Version 3. 