// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "blurhashimageprovider.h"

#include <QImage>
#include <QString>

#include "blurhash.hpp"

/*
 * Qt unfortunately re-encodes the base83 string in QML.
 * The only special ASCII characters used in the blurhash base83 string are:
 * #$%*+,-.:;=?@[]^_{|}~
 * QUrl::fromPercentEncoding is too greedy, and spits out invalid characters
 * for parts of valid base83 like %14.
 */
// clang-format off
static const QMap<QLatin1String, QLatin1String> knownEncodings = {
    {QLatin1String("%23A"), QLatin1String(":")},
    {QLatin1String("%3F"), QLatin1String("?")},
    {QLatin1String("%23"), QLatin1String("#")},
    {QLatin1String("%5B"), QLatin1String("[")},
    {QLatin1String("%5D"), QLatin1String("]")},
    {QLatin1String("%40"), QLatin1String("@")},
    {QLatin1String("%24"), QLatin1String("$")},
    {QLatin1String("%2A"), QLatin1String("*")},
    {QLatin1String("%2B"), QLatin1String("+")},
    {QLatin1String("%2C"), QLatin1String(",")},
    {QLatin1String("%2D"), QLatin1String("-")},
    {QLatin1String("%2E"), QLatin1String(".")},
    {QLatin1String("%3D"), QLatin1String("=")},
    {QLatin1String("%25"), QLatin1String("%")},
    {QLatin1String("%5E"), QLatin1String("^")},
    {QLatin1String("%7C"), QLatin1String("|")},
    {QLatin1String("%7B"), QLatin1String("{")},
    {QLatin1String("%7D"), QLatin1String("}")},
    {QLatin1String("%7E"), QLatin1String("~")},
};
// clang-format on

class AsyncImageResponseRunnable : public QObject, public QRunnable
{
    Q_OBJECT

Q_SIGNALS:
    void done(QImage image);

public:
    AsyncImageResponseRunnable(const QString &id, const QSize &requestedSize)
        : m_id(id)
        , m_requestedSize(requestedSize)
    {
        if (m_requestedSize.width() == -1) {
            m_requestedSize.setWidth(64);
        }
        if (m_requestedSize.height() == -1) {
            m_requestedSize.setHeight(64);
        }
    }

    void run() override
    {
        if (m_id.isEmpty()) {
            return;
        }

        QString decodedId = m_id;

        QMap<QLatin1String, QLatin1String>::const_iterator i;
        for (i = knownEncodings.constBegin(); i != knownEncodings.constEnd(); ++i)
            decodedId.replace(i.key(), i.value());

        auto data = blurhash::decode(decodedId.toLatin1().constData(), m_requestedSize.width(), m_requestedSize.height());
        QImage image(data.image.data(), static_cast<int>(data.width), static_cast<int>(data.height), static_cast<int>(data.width * 3), QImage::Format_RGB888);

        Q_EMIT done(image.convertToFormat(QImage::Format_RGB32));
    }

private:
    QString m_id;
    QSize m_requestedSize;
};

AsyncImageResponse::AsyncImageResponse(const QString &id, const QSize &requestedSize, QThreadPool *pool)
{
    auto runnable = new AsyncImageResponseRunnable(id, requestedSize);
    connect(runnable, &AsyncImageResponseRunnable::done, this, &AsyncImageResponse::handleDone);
    pool->start(runnable);
}

void AsyncImageResponse::handleDone(QImage image)
{
    m_image = image;
    Q_EMIT finished();
}

QQuickTextureFactory *AsyncImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QQuickImageResponse *BlurhashImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    return new AsyncImageResponse(id, requestedSize, &pool);
}

#include "blurhashimageprovider.moc"
