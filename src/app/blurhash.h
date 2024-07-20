// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <QtGui/QImage>

namespace Quotient
{
/**
 * @brief Encodes and decodes image to and from the BlurHash format. See https://blurha.sh/.
 */
namespace BlurHash
{
/** Decodes the @p blurhash string creating an image of @p size.
 * @note Returns a null image if decoding failed.
 */
QImage decode(const QString &blurhash, const QSize &size);

/** Encodes the @p image and returns a blurhash string.
 * @param image A non-null image.
 * @param componentsX the number of components X-wise. Must be between 1 and 9.
 * @param componentsY the number of components Y-wise. Must be between 1 and 9.
 * @note Returns an empty string if it failed to encode the image.
 */
QString encode(const QImage &image, int componentsX = 4, int componentsY = 4);
} // namespace BlurHash
} // namespace Quotient
