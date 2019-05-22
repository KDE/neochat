#include "imageclipboard.h"

#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QUrl>
#include <QtDebug>

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

bool ImageClipboard::saveImage(const QUrl& localPath) {
  if (!localPath.isLocalFile())
    return false;

  auto i = image();

  if (i.isNull())
    return false;

  QString path = QFileInfo(localPath.toLocalFile()).absolutePath();
  QDir dir;
  if (!dir.exists(path))
    dir.mkpath(path);

  i.save(localPath.toLocalFile());

  return true;
}
