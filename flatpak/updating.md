<!-- SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
     SPDX-License-Identifier: CC0-1.0 -->
     
# Updating rust flatpak dependencies

Assuming libQuotient sources in a folder next to the neochat sources and `https://raw.githubusercontent.com/flatpak/flatpak-builder-tools/refs/heads/master/cargo/flatpak-cargo-generator.py` in the the user's home folder:

1. In `libquotient/Quotient/crypto-sdk`:
    ```
    cargo update
    ```
2. In `neochat`
    ```
    ~/flatpak-cargo-generator.py ../libquotient/Quotient/crypto-sdk/Cargo.lock -o flatpak/generated-sources.json
    ```
