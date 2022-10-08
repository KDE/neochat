// SPDX-FileCopyrightText: 2018 Wolt Enterprises
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>

uint8_t *decode(const char *blurhash, int width, int height, int punch, int nChannels);

bool isValidBlurhash(const char *blurhash);
