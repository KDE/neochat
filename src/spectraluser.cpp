#include "spectraluser.h"

QColor SpectralUser::color()
{
    return QColor::fromHslF(hueF(), 0.7, 0.5, 1);
}
