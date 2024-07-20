// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: MIT

#include "blurhash.h"

#include <QtGui/QColorSpace>

#include <numbers>

// From https://github.com/woltapp/blurhash/blob/master/Algorithm.md#base-83
const static QString b83Characters{QStringLiteral("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#$%*+,-.:;=?@[]^_{|}~")};

const static auto toLinearSRGB = QColorSpace(QColorSpace::SRgb).transformationToColorSpace(QColorSpace::SRgbLinear);
const static auto fromLinearSRGB = QColorSpace(QColorSpace::SRgbLinear).transformationToColorSpace(QColorSpace::SRgb);

using namespace Quotient;
using Components = std::pair<int, int>;

std::optional<int> decode83(const QString &encodedString)
{
    int temp = 0;
    for (const QChar c : encodedString) {
        const auto index = b83Characters.indexOf(c);
        if (index == -1)
            return std::nullopt;

        temp = temp * 83 + static_cast<int>(index);
    }

    return temp;
}

QString encode83(int value)
{
    QString buffer;

    do {
        buffer += b83Characters[value % 83];
    } while ((value = value / 83));

    std::ranges::reverse(buffer);

    return buffer;
}

Components unpackComponents(const int packedComponents)
{
    return {packedComponents % 9 + 1, packedComponents / 9 + 1};
}

int packComponents(const Components &components)
{
    const auto [componentX, componentY] = components;
    return (componentX - 1) + (componentY - 1) * 9;
}

float decodeMaxAC(const int value)
{
    return static_cast<float>(value + 1) / 166.f;
}

int encodeMaxAC(const float value)
{
    return std::clamp(static_cast<int>(value * 166 - 0.5f), 0, 82);
}

QColor decodeAverageColor(const int encodedValue)
{
    const int intR = encodedValue >> 16;
    const int intG = (encodedValue >> 8) & 255;
    const int intB = encodedValue & 255;

    return QColor::fromRgb(intR, intG, intB);
}

int encodeAverageColor(const QColor &averageColor)
{
    return (averageColor.red() << 16) + (averageColor.green() << 8) + averageColor.blue();
}

float signPow(const float value, const float exp)
{
    return std::copysign(std::pow(std::abs(value), exp), value);
}

QColor decodeAC(const int value, const float maxAC)
{
    const auto quantR = value / (19 * 19);
    const auto quantG = (value / 19) % 19;
    const auto quantB = value % 19;

    return QColor::fromRgbF(signPow((static_cast<float>(quantR) - 9) / 9, 2) * maxAC,
                            signPow((static_cast<float>(quantG) - 9) / 9, 2) * maxAC,
                            signPow((static_cast<float>(quantB) - 9) / 9, 2) * maxAC);
}

int encodeAC(const QColor value, const float maxAC)
{
    const auto quantR = static_cast<int>(std::max(0., std::min(18., std::floor(signPow(value.redF() / maxAC, 0.5) * 9 + 9.5))));
    const auto quantG = static_cast<int>(std::max(0., std::min(18., std::floor(signPow(value.greenF() / maxAC, 0.5) * 9 + 9.5))));
    const auto quantB = static_cast<int>(std::max(0., std::min(18., std::floor(signPow(value.blueF() / maxAC, 0.5) * 9 + 9.5))));

    return quantR * 19 * 19 + quantG * 19 + quantB;
}

QList<float> calculateWeights(const qsizetype dimension, const qsizetype components)
{
    QList<float> bases(dimension * components, 0.0f);

    const auto scale = static_cast<float>(std::numbers::pi) / static_cast<float>(dimension);
    for (qsizetype x = 0; x < dimension; x++) {
        for (qsizetype nx = 0; nx < components; nx++) {
            bases[x * components + nx] = std::cos(scale * static_cast<float>(nx * x));
        }
    }
    return bases;
}

QImage BlurHash::decode(const QString &blurhash, const QSize &size)
{
    // 10 is the minimum length of a blurhash string
    if (blurhash.length() < 10)
        return {};

    // First character is the number of components
    const auto components83 = decode83(blurhash.first(1));
    if (!components83.has_value())
        return {};

    const auto [componentX, componentY] = unpackComponents(*components83);
    const auto minimumSize = 1 + 1 + 4 + (componentX * componentY - 1) * 2;
    if (componentX < 1 || componentY < 1 || blurhash.size() != minimumSize)
        return {};

    // Second character is the maximum AC component value
    const auto maxAC83 = decode83(blurhash.mid(1, 1));
    if (!maxAC83.has_value())
        return {};

    const auto maxAC = decodeMaxAC(*maxAC83);

    // Third character onward is the average color of the image
    const auto averageColor83 = decode83(blurhash.mid(2, 4));
    if (!averageColor83.has_value())
        return {};

    const auto averageColor = toLinearSRGB.map(decodeAverageColor(*averageColor83));

    QList values = {averageColor};

    // Iterate through the rest of the string for the color values
    // Each AC component is two characters each
    for (qsizetype c = 6; c < blurhash.size(); c += 2) {
        const auto acComponent83 = decode83(blurhash.mid(c, 2));
        if (!acComponent83.has_value())
            return {};

        values.append(decodeAC(*acComponent83, maxAC));
    }

    QImage image(size, QImage::Format_RGB888);
    image.setColorSpace(QColorSpace::SRgb);

    const auto basisX = calculateWeights(size.width(), componentX);
    const auto basisY = calculateWeights(size.height(), componentY);

    for (int y = 0; y < size.height(); y++) {
        for (int x = 0; x < size.width(); x++) {
            float linearSumR = 0.0f;
            float linearSumG = 0.0f;
            float linearSumB = 0.0f;

            for (int nx = 0; nx < componentX; nx++) {
                for (int ny = 0; ny < componentY; ny++) {
                    const float basis = basisX[x * componentX + nx] * basisY[y * componentY + ny];

                    linearSumR += values[nx + ny * componentX].redF() * basis;
                    linearSumG += values[nx + ny * componentX].greenF() * basis;
                    linearSumB += values[nx + ny * componentX].blueF() * basis;
                }
            }

            auto linearColor = QColor::fromRgbF(linearSumR, linearSumG, linearSumB);
            image.setPixelColor(x, y, fromLinearSRGB.map(linearColor));
        }
    }

    return image;
}

QString BlurHash::encode(const QImage &image, const int componentsX, const int componentsY)
{
    Q_ASSERT(componentsX >= 1 && componentsX <= 9);
    Q_ASSERT(componentsY >= 1 && componentsY <= 9);

    if (image.isNull())
        return {};

    const auto basisX = calculateWeights(image.width(), componentsX);
    const auto basisY = calculateWeights(image.height(), componentsY);

    QList<QColor> factors;
    factors.resize(componentsX * componentsY);

    const float normalizationFactor = 1.0f / static_cast<float>(image.width());

    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            const QColor srgbColor = image.pixelColor(x, y);
            const QColor linearColor = toLinearSRGB.map(srgbColor);

            float linearR = linearColor.redF();
            float linearG = linearColor.greenF();
            float linearB = linearColor.blueF();

            linearR *= normalizationFactor;
            linearG *= normalizationFactor;
            linearB *= normalizationFactor;

            for (int ny = 0; ny < componentsY; ny++) {
                for (int nx = 0; nx < componentsX; nx++) {
                    const float basis = basisX[x * componentsX + nx] * basisY[y * componentsY + ny];

                    float factorR = factors[ny * componentsX + nx].redF();
                    float factorG = factors[ny * componentsX + nx].greenF();
                    float factorB = factors[ny * componentsX + nx].blueF();

                    factors[ny * componentsX + nx] = QColor::fromRgbF(factorR + linearR * basis, factorG + linearG * basis, factorB + linearB * basis);
                }
            }
        }
    }

    // Scale by normalization. Half the scaling is done in the previous loop to prevent going
    // too far outside the float range.
    for (qsizetype i = 0; i < factors.size(); i++) {
        float normalisation = (i == 0) ? 1 : 2;
        float scale = normalisation / static_cast<float>(image.height());

        float factorR = factors[i].redF() * scale;
        float factorG = factors[i].greenF() * scale;
        float factorB = factors[i].blueF() * scale;

        factors[i] = QColor::fromRgbF(factorR, factorG, factorB);
    }

    const auto averageColor = factors.takeFirst();

    QString encodedString;
    encodedString.append(encode83(packComponents(Components(componentsX, componentsY))).rightJustified(1, QLatin1Char('0')));

    float maximumValue;
    if (!factors.empty()) {
        float actualMaximumValue = 0;
        for (auto ac : factors) {
            actualMaximumValue = std::max({
                std::abs(ac.redF()),
                std::abs(ac.greenF()),
                std::abs(ac.blueF()),
                actualMaximumValue,
            });
        }

        int quantisedMaximumValue = encodeMaxAC(actualMaximumValue);
        maximumValue = (static_cast<float>(quantisedMaximumValue) + 1) / 166;
        encodedString.append(encode83(quantisedMaximumValue).leftJustified(1, QLatin1Char('0')));
    } else {
        maximumValue = 1;
        encodedString.append(encode83(0).leftJustified(1, QLatin1Char('0')));
    }

    encodedString.append(encode83(encodeAverageColor(fromLinearSRGB.map(averageColor))).leftJustified(4, QLatin1Char('0')));

    for (auto ac : factors)
        encodedString.append(encode83(encodeAC(ac, maximumValue)).leftJustified(2, QLatin1Char('0')));

    return encodedString;
}
