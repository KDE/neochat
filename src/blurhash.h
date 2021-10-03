// SPDX-FileCopyrightText: 2018 Wolt Enterprises
// SPDX-License-Identifier: MIT

#pragma once

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint8_t *decode(const char *blurhash, int width, int height, int punch, int nChannels);

bool isValidBlurhash(const char *blurhash);
