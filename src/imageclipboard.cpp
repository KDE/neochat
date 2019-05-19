#include "imageclipboard.h"

#include <QGuiApplication>
#include <QUrl>

ImageClipboard::ImageClipboard(QObject* parent)
    : QObject(parent), m_clipboard(QGuiApplication::clipboard()) {
  connect(m_clipboard, &QClipboard::changed, this,
          &ImageClipboard::imageChanged);
}

bool ImageClipboard::hasImage() {
  return !image().isNull();
}

QImage ImageClipboard::image() {
  return m_clipboard->image();
}

void ImageClipboard::saveImage(const QUrl& localPath) {
    auto i = image();

    if (i.isNull()) return;

    i.save(localPath.toString());
}
