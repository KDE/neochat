// SPDX-FileCopyrightText: 2018 Wolt Enterprises
// SPDX-License-Identifier: MIT

#pragma once

#include <stdint.h>

/**
 * @brief Returns the pixel array of the result image given the blurhash string.
 *
 * @param blurhash a string representing the blurhash to be decoded.
 * @param width the width of the resulting image.
 * @param height the height of the resulting image.
 * @param punch the factor to improve the contrast, default = 1.
 * @param nChannels the number of channels in the resulting image array, 3 = RGB, 4 = RGBA.
 *
 * @return A pointer to memory region where pixels are stored in (H, W, C) format.
 */
uint8_t *decode(const char *blurhash, int width, int height, int punch, int nChannels);

/**
 * @brief Checks if the Blurhash is valid or not.
 *
 * @param blurhash a string representing the blurhash.
 *
 * @return A bool (true if it is a valid blurhash, else false).
 */
bool isValidBlurhash(const char *blurhash);
