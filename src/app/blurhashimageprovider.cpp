// SPDX-FileCopyrightText: 2024 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: MIT

#include "blurhashimageprovider.h"

#include <Quotient/blurhash.h>

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
        if (m_requestedSize.width() == -1)
            m_requestedSize.setWidth(64);
        if (m_requestedSize.height() == -1)
            m_requestedSize.setHeight(64);
    }

    void run() override
    {
        if (m_id.isEmpty())
            return;

        QString decodedId = m_id;
        for (auto i = knownEncodings.constBegin(); i != knownEncodings.constEnd(); ++i)
            decodedId.replace(i.key(), i.value());

        Q_EMIT done(Quotient::BlurHash::decode(decodedId, m_requestedSize));
    }

private:
    QString m_id;
    QSize m_requestedSize;
};

AsyncImageResponse::AsyncImageResponse(const QString &id, const QSize &requestedSize, QThreadPool *pool)
{
    const auto runnable = new AsyncImageResponseRunnable(id, requestedSize);
    connect(runnable, &AsyncImageResponseRunnable::done, this, &AsyncImageResponse::handleDone);
    pool->start(runnable);
}

void AsyncImageResponse::handleDone(QImage image)
{
    m_image = std::move(image);
    Q_EMIT finished();
}

QQuickTextureFactory *AsyncImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(m_image);
}

QQuickImageResponse *BlurHashImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    return new AsyncImageResponse(id, requestedSize, &pool);
}

#include "blurhashimageprovider.moc"
