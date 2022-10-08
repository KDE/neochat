// SPDX-FileCopyrightText: 2018 Wolt Enterprises
// SPDX-License-Identifier: MIT

#include "blurhash.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

namespace
{
const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#$%*+,-.:;=?@[]^_{|}~";

struct Color {
    float r = 0;
    float g = 0;
    float b = 0;
};

inline int linearTosRGB(float value)
{
    float v = fmaxf(0, fminf(1, value));
    if (v <= 0.0031308)
        return v * 12.92 * 255 + 0.5;
    else
        return (1.055 * powf(v, 1 / 2.4) - 0.055) * 255 + 0.5;
}

inline float sRGBToLinear(int value)
{
    float v = (float)value / 255;
    if (v <= 0.04045)
        return v / 12.92;
    else
        return powf((v + 0.055) / 1.055, 2.4);
}

inline float signPow(float value, float exp)
{
    return copysignf(powf(fabsf(value), exp), value);
}

inline uint8_t clampToUByte(int *src)
{
    if (*src >= 0 && *src <= 255) {
        return *src;
    }
    return (*src < 0) ? 0 : 255;
}

inline uint8_t *createByteArray(int size)
{
    return (uint8_t *)malloc(size * sizeof(uint8_t));
}

int decodeToInt(const char *string, int start, int end)
{
    int value = 0;
    for (int iter1 = start; iter1 < end; iter1++) {
        int index = -1;
        for (int iter2 = 0; iter2 < 83; iter2++) {
            if (chars[iter2] == string[iter1]) {
                index = iter2;
                break;
            }
        }
        if (index == -1) {
            return -1;
        }
        value = value * 83 + index;
    }
    return value;
}

void decodeDC(int value, Color *color)
{
    color->r = sRGBToLinear(value >> 16);
    color->g = sRGBToLinear((value >> 8) & 255);
    color->b = sRGBToLinear(value & 255);
}

void decodeAC(int value, float maximumValue, Color *color)
{
    int quantR = (int)floorf(value / (19 * 19));
    int quantG = (int)floorf(value / 19) % 19;
    int quantB = (int)value % 19;

    color->r = signPow(((float)quantR - 9) / 9, 2.0) * maximumValue;
    color->g = signPow(((float)quantG - 9) / 9, 2.0) * maximumValue;
    color->b = signPow(((float)quantB - 9) / 9, 2.0) * maximumValue;
}

int decodeToArray(const char *blurhash, int width, int height, int punch, int nChannels, uint8_t *pixelArray)
{
    if (!isValidBlurhash(blurhash)) {
        return -1;
    }
    if (punch < 1) {
        punch = 1;
    }

    int sizeFlag = decodeToInt(blurhash, 0, 1);
    int numY = (int)floorf(sizeFlag / 9) + 1;
    int numX = (sizeFlag % 9) + 1;
    int iter = 0;

    Color color;
    int quantizedMaxValue = decodeToInt(blurhash, 1, 2);
    if (quantizedMaxValue == -1) {
        return -1;
    }

    const float maxValue = ((float)(quantizedMaxValue + 1)) / 166;

    const int colors_size = numX * numY;

    std::vector<Color> colors(colors_size, {0, 0, 0});

    for (iter = 0; iter < colors_size; iter++) {
        if (iter == 0) {
            int value = decodeToInt(blurhash, 2, 6);
            if (value == -1) {
                return -1;
            }
            decodeDC(value, &color);
            colors[iter] = color;
        } else {
            int value = decodeToInt(blurhash, 4 + iter * 2, 6 + iter * 2);
            if (value == -1) {
                return -1;
            }
            decodeAC(value, maxValue * punch, &color);
            colors[iter] = color;
        }
    }

    int bytesPerRow = width * nChannels;
    int x = 0, y = 0, i = 0, j = 0;
    int intR = 0, intG = 0, intB = 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0;

            for (j = 0; j < numY; j++) {
                for (i = 0; i < numX; i++) {
                    float basics = cos((M_PI * x * i) / width) * cos((M_PI * y * j) / height);
                    int idx = i + j * numX;
                    r += colors[idx].r * basics;
                    g += colors[idx].g * basics;
                    b += colors[idx].b * basics;
                }
            }

            intR = linearTosRGB(r);
            intG = linearTosRGB(g);
            intB = linearTosRGB(b);

            pixelArray[nChannels * x + 0 + y * bytesPerRow] = clampToUByte(&intR);
            pixelArray[nChannels * x + 1 + y * bytesPerRow] = clampToUByte(&intG);
            pixelArray[nChannels * x + 2 + y * bytesPerRow] = clampToUByte(&intB);

            if (nChannels == 4) {
                pixelArray[nChannels * x + 3 + y * bytesPerRow] = 255;
            }
        }
    }
    return 0;
}
}

uint8_t *decode(const char *blurhash, int width, int height, int punch, int nChannels)
{
    int bytesPerRow = width * nChannels;
    uint8_t *pixelArray = createByteArray(bytesPerRow * height);

    if (decodeToArray(blurhash, width, height, punch, nChannels, pixelArray) == -1) {
        return NULL;
    }
    return pixelArray;
}

bool isValidBlurhash(const char *blurhash)
{
    const int hashLength = strlen(blurhash);

    if (!blurhash || strlen(blurhash) < 6) {
        return false;
    }

    int sizeFlag = decodeToInt(blurhash, 0, 1);
    int numY = (int)floorf(sizeFlag / 9) + 1;
    int numX = (sizeFlag % 9) + 1;

    return hashLength == 4 + 2 * numX * numY;
}
